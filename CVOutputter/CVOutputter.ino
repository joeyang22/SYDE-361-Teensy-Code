#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


int pitchPin = A7;
int volumePin = A6;
AudioSynthWaveformDc dc0;
AudioSynthWaveformDc dc1;

//AudioOutputAnalog dac0(0);
//AudioOutputAnalog dac1(1);
//
//AudioConnection patchCord(dc0,dac0);
//AudioConnection patchCord(dc1,dac1);

void setup() {
  AudioMemory(12);
  Serial.begin(9600);
  pinMode(pitchPin, INPUT);
  pinMode(volumePin, INPUT);
}

void loop() {
  int pitchRead = pulseIn(pitchPin, HIGH);
  Serial.print("Pitch: ");
  Serial.println(pitchRead);
  int volRead = pulseIn(volumePin, HIGH);
  Serial.print("Volume: ");
  Serial.println(volRead);
//  int volOn = 0;
//  if (volRead > 0) {
//    volOn = 1;
//  }
//  dc0.amplitude(volOn);
//  dc1.amplitude(pitchRead/1023.0);
//  
  delay(100);
}
