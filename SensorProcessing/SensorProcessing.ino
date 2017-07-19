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
float diff = max_distance - min_distance;
float num_of_registers = 2;
float max_note = 6;
float min_note = 0;
int intval = (max_distance- min_distance) / (max_note - min_note);
int note = int(min_note);
int channel = 1; // Defines the MIDI channel to send messages on (values from 1-16)
int prev_note = int(min_note);
float avg = 0.0;
int buzz_max = 0;
const int WINDOW_LENGTH = 10; // this number times 25ms is the time averaged over
float inputVolume = 0;
 
void setup()
{
  delay(250);
  AudioMemory(30);
  delay(250);

  
  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
//  audioShield.volume(0.5);

  pinMode(A6, OUTPUT);
  notefreq.begin(0.15); 
  
   
  Serial.begin(9600);               // starts the serial monitor
}

int getSlideNote(float distance) {
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
  buzz_max = 0;
  convertedDistance = 0;
  // Get an average
  int mouthPieceRegister = 0;
  float floatMouthPieceRegister = 0;
  inputVolume = 0;
  if (notefreq.available() && notevolume.available()) {
    inputVolume = notevolume.readPeakToPeak();
    for (int i = 0; i < WINDOW_LENGTH; i++) {
      int freq = notefreq.read();
      if (freq > buzz_max && freq < 600){
        buzz_max = freq;
      }

      val = analogRead(A3) * 0.0048828125;
      convertedDistance +=  65*pow(val, -1.10);
      delay(5);
    }
  }

  if (inputVolume > 1.5) {
    if (buzz_max < 150) {
      mouthPieceRegister = 50;
      floatMouthPieceRegister = 0;
    } else {
      mouthPieceRegister = 57;
      floatMouthPieceRegister = 1;
    }
  } 
  
  
  if (mouthPieceRegister != 0) {
    digitalWrite(A6, HIGH);
    int temp = convertedDistance/WINDOW_LENGTH;
    note = getSlideNote(temp);
    temp = (temp - min_distance) / intval;    
    float floteNote = float(convertedDistance) / float(WINDOW_LENGTH) - 12.0;
    floteNote = min(diff, max(0, diff - floteNote));
    floteNote = (floteNote + diff*floatMouthPieceRegister)*(255.0)/(diff*num_of_registers);
    

      note = note + mouthPieceRegister;
      if (note != prev_note)
      {
        analogWrite(A14, floteNote*(1.17/3.3));

        usbMIDI.sendNoteOff(prev_note, 0, channel);
        prev_note = note;
        float velocity = (analogRead(A7) / 1024.0) * 127.0;
        usbMIDI.sendNoteOn(note, velocity, channel);
      }
  } else {
    digitalWrite(A6, LOW);
    usbMIDI.sendNoteOff(prev_note, 0, channel);
    prev_note = 0;
  }
  delay(50);
} 
