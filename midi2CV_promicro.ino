#define  MIDI_RXCH               10
#define  PULSE_LED_LENGHT        100 // in uSeconds
#define  PULSE_CLOCK_OUT_LENGHT  10 //in uSeconds

// HARDWARE
#define  CLOCK_OUT   18
#define  GATE_OUT    15
#define  LED_OUT     14
#define  AUX1_IO     16
#define  AUX2_IO     10

#define  CS_4822     4
#define  LDAC_4822   5

#include <elapsedMillis.h>

#include <MCP48xx.h>
// Define the MCP4822 instance, giving it the SS (Slave Select) pin
// The constructor will also initialize the SPI library
// We can also define a MCP4812 or MCP4802
MCP4822 dac(CS_4822);

#include <MIDI.h>

//MIDI_CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

uint8_t  midiClock_counter = 0;

elapsedMillis  TS_pulseLED = 0;
elapsedMillis  TS_clockOut = 0;

// -----------------------------------------------------------------------------

// see documentation here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do whatever you want when a note is pressed.

    // Try to keep your callbacks short (no delays ect)
    // otherwise it would slow down the loop() and have a bad impact
    // on real-time performance.
    dac.setVoltageA(pitch * 8);
    dac.setVoltageB(velocity * 8);
    digitalWrite(GATE_OUT, true);

    pulseLED( true );

}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
    dac.setVoltageA(pitch * 8);
    dac.setVoltageB(velocity * 8);
    digitalWrite(GATE_OUT, false);

    pulseLED( true );
}

void handleClock(void)
{
    midiClock_counter++;
    
    if( midiClock_counter >= 24 )
    {
         midiClock_counter = 0;
         updateClockOut( true );
    }

    pulseLED( true );
}

void updateClockOut( boolean flagCLK )
{
   if( flagCLK ) TS_clockOut = 0 ;
   
   if( TS_clockOut > PULSE_CLOCK_OUT_LENGHT )
   {
      digitalWrite( CLOCK_OUT, false );
   }else{
      digitalWrite( CLOCK_OUT, true );
   }

}

void handleStart(void)
{
      midiClock_counter = 23; //last 
      pulseLED( true );
}

void handleContinue(void)
{
     pulseLED( true );
}

void handleStop(void)
{
    dac.setVoltageB(0);
    digitalWrite(GATE_OUT, false);
    digitalWrite(CLOCK_OUT, false);

    pulseLED( true );
}

void pulseLED( boolean flagPulseLed )
{
   if( flagPulseLed ) TS_pulseLED = 0 ;
   
   if( TS_pulseLED > PULSE_LED_LENGHT )
   {
      digitalWrite( LED_OUT, false );
   }else{
      digitalWrite( LED_OUT, true );
   }

}


// -----------------------------------------------------------------------------

void setup()
{
    pinMode(CLOCK_OUT, OUTPUT);
    digitalWrite(CLOCK_OUT, LOW);

    pinMode(GATE_OUT , OUTPUT);
    digitalWrite(GATE_OUT , LOW);

    pinMode(LED_OUT, OUTPUT);
    digitalWrite(LED_OUT, LOW);

    pinMode(AUX1_IO , OUTPUT);
    digitalWrite(AUX1_IO , LOW);

    pinMode(AUX2_IO , OUTPUT);
    digitalWrite(AUX2_IO , LOW);

    pinMode(LDAC_4822 , OUTPUT);
    digitalWrite(LDAC_4822 , LOW);

    
    // We call the init() method to initialize the instance

    dac.init();

    // The channels are turned off at startup so we need to turn the channel we need on
    dac.turnOnChannelA();
    dac.turnOnChannelB();

    // We configure the channels in High gain
    // It is also the default value so it is not really needed
    dac.setGainA(MCP4822::High);
    dac.setGainB(MCP4822::High);
    
    
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);

    MIDI.setHandleClock(handleClock);
    MIDI.setHandleStart(handleStart);
    MIDI.setHandleContinue(handleContinue);
    MIDI.setHandleStop(handleStop);

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_RXCH); //(MIDI_CHANNEL_OMNI);
}

void loop()
{
      // Call MIDI.read the fastest you can for real-time performance.
      MIDI.read();
    
      // We send the command to the MCP4822
      // This is needed every time we make any change
      dac.updateDAC();
    
      //update Clock Out
      updateClockOut( false );

      // update led
      pulseLED( false );
}




//EOF
