#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


int volumePin = A7;
int onOffPin = 20;

void setup() {
  AudioMemory(12);
  Serial.begin(9600);

  pinMode(onOffPin, INPUT);
}

void loop() {
  int volRead = analogRead(volumePin);
  int isOn = digitalRead(onOffPin);
  
  if (isOn == LOW) {
    volRead = 0;
  }
  analogWrite(A22, volRead*255.0/1024.0);
}
