// TODO
// Fix tonguing triggers high frequency buzz at the end of a note 
// 

#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

const int myInput = AUDIO_INPUT_LINEIN;
 
// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
//
AudioInputI2S     audioInput; 
AudioAnalyzeNoteFrequency notefreq;
AudioAnalyzePeak notevolume;
 
// Connect either the live input or synthesized sine wave
AudioConnection patchCord1(audioInput, 0, notefreq, 0);
AudioConnection patchCord2(audioInput, 0, notevolume, 0);

AudioControlSGTL5000 audioShield;
 
double val = 0;                 // variable to store the values from sensor(initially zero)
float convertedDistance = 0;
float max_distance = 33;
float min_distance = 12;
float max_note = 6;
float min_note = 0;
int intval = (max_distance- min_distance) / (max_note - min_note);
int note = int(min_note);
int channel = 1; // Defines the MIDI channel to send messages on (values from 1-16)
int velocity = 100; // Defines the velocity that the note plays at (values from 0-127)
int prev_note = int(min_note);
float avg = 0.0;
int buzz_max = 0;
const int WINDOW_LENGTH = 10; // this number times 25ms is the time averaged over
 
void setup()
{
  delay(250);
  AudioMemory(30);
  delay(250);

  
  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
//  audioShield.volume(0.5);
  
  notefreq.begin(0.15); 
//  pinMode(A6, OUTPUT);
//  pinMode(A7, OUTPUT);
   
  Serial.begin(9600);               // starts the serial monitor
}
 
void loop()
{
  buzz_max = 0;
  convertedDistance = 0;
  // Get an average
  int mouthPieceRegister = 0;
  if (notefreq.available() && notevolume.available()) {
    float  inputVolume= notevolume.readPeakToPeak();
    for (int i = 0; i < WINDOW_LENGTH; i++) {
      int freq = notefreq.read();
      if (freq > buzz_max && freq < 600){
        buzz_max = freq;
      }
      val = analogRead(A3) * 0.0048828125;
      convertedDistance +=  65*pow(val, -1.10);
      delay(5);
    }
//    Serial.println(buzz_max);
//    Serial.println(convertedDistance/WINDOW_LENGTH);
//    mouthPieceRegister = 50;
  }
  Serial.println(buzz_max);
//    if (inputVolume > 1.5) {
//      if (buzz_max < 225) {

//      } else {
//        mouthPieceRegister = 67;
//      }
//    } else {
//    }
//  } 
  
  
  if (mouthPieceRegister != 0) {
    int temp = convertedDistance/WINDOW_LENGTH;
    temp = (temp - min_distance) / intval;    
    note = min(6, max(0, 6-temp));
    if (note >= min_note && note <= max_note) {
      note = note + mouthPieceRegister;
      if (note != prev_note)
      {
        usbMIDI.sendNoteOff(prev_note, 0, channel);
        prev_note = note;
//        Serial.println("yay");
        usbMIDI.sendNoteOn(note, velocity, channel);
      }
    } else {
      usbMIDI.sendNoteOff(prev_note, 0, channel);
    }
  }
  else {
    usbMIDI.sendNoteOff(prev_note, 0, channel);
    prev_note = 0;
  }
  delay(50);
}
