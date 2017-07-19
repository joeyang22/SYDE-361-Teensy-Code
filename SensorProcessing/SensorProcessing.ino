// Imports
#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

const int myInput = AUDIO_INPUT_LINEIN;
 
// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
AudioInputI2S     audioInput; 
AudioAnalyzeNoteFrequency notefreq;
AudioAnalyzePeak notevolume;
 
// Connect either the live input or synthesized sine wave
AudioConnection patchCord1(audioInput, 0, notefreq, 0);
AudioConnection patchCord2(audioInput, 0, notevolume, 0);

// Set up the audio chield
AudioControlSGTL5000 audioShield;

/*
 * GLOBAL VARIABLES
 */

double val = 0;               // variable to store the values from sensor(initially zero)
float convertedDistance = 0;  // A distance value that is processed from the IR sensor 
float num_of_registers = 2;   // How many registers are possible
int note = 0;                 // which note to play
int prev_note = 0;            // Indicates which note was last played (So we can turn it off for MIDI output)
int channel = 1;              // Defines the MIDI channel to send messages on (values from 1-16)
int buzz_max = 0;             // Peak frequency read over values
const int WINDOW_LENGTH = 10; // this number times 5ms is the time IR sensor and Microphone sensor readings are processed over
float inputVolume = 0;        // How loud the reading from the microphone was.
float maxDistance = 40.0;     // Roughly the maximum distance a user extends their arm
float minDistance = 12.0;     // The minimum distance the sensor can read
float diff = maxDistance - minDistance; // The difference between max and min difference
/*
 * Setup code
 */
void setup()
{
  // Setup audio memory
  delay(250);
  AudioMemory(30);
  delay(250);

  // Enable the audio shield and set the input
  audioShield.enable();
  audioShield.inputSelect(myInput);

  // Initialize A6 pin as an output pin so we can send gate on/off to the second teensy.
  pinMode(A6, OUTPUT);
  // Initialize notefreq to determine what note is being played
  notefreq.begin(0.15); 
  // Begin the serial monitor
  Serial.begin(9600);
}

/*
 * Main Functionality
 */

/*
 * Determines which midi note maps to a specific distance. This corresponds to real life slide positions of a tenor trombone.
 * 
 * @param distance The distance in cm that should be converted to a note
 * @return A value from 0-6 (Has a range of 7 semitones)
 */
int getSlideNote(float distance) {
  // The further away, the lower the note is.
  if (distance >= 60.5) {
    return 0;
  } else if (distance >= 51.1) {
    return 1;
  } else if (distance >= 42) {
    return 2;
  } else if (distance >= 33.8) {
    return 3;
  } else if (distance >= 26.14) {
    return 4;
  } else if (distance >= 18.71) {
    return 5;
  } else {
    return 6;
  }
}


void loop()
{
  // Reset all variables that need to be initialized.
  buzz_max = 0;
  convertedDistance = 0;
  int mouthPieceRegister = 0;
  float floatMouthPieceRegister = 0;
  inputVolume = 0;

  // Check if the microphone is available to read frequencies & volume.
  if (notefreq.available() && notevolume.available()) {
    // Read the peak to peak volume of the note
    inputVolume = notevolume.readPeakToPeak();
    // Take an average over WINDOW_LENGTH intervals for the distance. Takes a maximum value of the buzzing frequency.
    for (int i = 0; i < WINDOW_LENGTH; i++) {
      // Take the maximum value of the frequency, but also limit the max frequency to 600 (Sometimes the mic randomly outputs high frequencies)
      int freq = notefreq.read();
      if (freq > buzz_max && freq < 600){
        buzz_max = freq;
      }

      // Convert the voltage reading from pin A3 to a distance in cm.
      val = analogRead(A3) * 0.0048828125;
      convertedDistance +=  65*pow(val, -1.10);
      delay(5);
    }
  }
  // Get the average distance
  int temp = convertedDistance/WINDOW_LENGTH;
  
  // Only play a sound if the buzzing was loud enough
  if (inputVolume > 1.5) {
    // Decide which register to play on - Depends on the frequency read above
    if (buzz_max < 150) {
      // Set the base MIDI value and specific register for analog output.
      mouthPieceRegister = 52; // E4, 1st position on this will be 58 which is a bflat
      floatMouthPieceRegister = 0;
    } else {
      mouthPieceRegister = 59; // Next register begins at +7 from the previous
      floatMouthPieceRegister = 1;
    }
  } 
  

  // If a note was detected, play a sound
  if (mouthPieceRegister != 0) {
    // Send on signal to the other teensy (For analog output)
    digitalWrite(A6, HIGH);

    // DIGITAL NOTE CALCULATIONS
    // Get the note based on the distance
    note = getSlideNote(temp);
    // Increase the note value by the baseline MIDI value
    note = note + mouthPieceRegister;

    // ANALOG NOTE CALCULATIONS
    // Determinine the control voltage based on the distance
    float floatNote = float(temp) - minDistance;
    // Clamp the values between 0 and the range of notes
    floatNote = min(diff, max(0, diff - floatNote));
    // Add distance according to which register we're at (multiplies the distance range by the amount of registers)
    floatNote = floatNote + diff*floathMouthPieceRegister;
    // Multiply the value by 255 to get the right analog output range.
    floatNote = floatNote * 255.0;
    // Divide it to get evenly spaced register to voltage intervals)
    floatNote = floatNote/ (diff *num_of_registers);
    // Write the analog signal. Modulate the value by the amount of octaves we have and the input voltage to the note.
    analogWrite(A14, floteNote*(1.17/3.3));
      
    // Check to see if a new note is being played
    if (note != prev_note)
    { 
      // Turn off the previous note
      usbMIDI.sendNoteOff(prev_note, 0, channel);
      prev_note = note;
      // Determine velocity from the signal pin A7
      float velocity = (analogRead(A7) / 1024.0) * 127.0;
      // Send the note on signal.
      usbMIDI.sendNoteOn(note, velocity, channel);
    }
  } else {
    // Send off signal to the other teensy
    digitalWrite(A6, LOW);
    // Turn off the previously sent note
    usbMIDI.sendNoteOff(prev_note, 0, channel);
    prev_note = 0;
  }
  delay(50);
} 
