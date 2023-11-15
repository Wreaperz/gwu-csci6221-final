#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "AudioSampleBugatti.h"  // Replace 'Yourfile' with the actual filename

// GUItool: begin automatically generated code
AudioPlayMemory          playMem1;       //xy=332,238
AudioOutputI2S           i2s1;           //xy=501,237
AudioConnection          patchCord1(playMem1, 0, i2s1, 0); // 0 = left
AudioConnection          patchCord2(playMem1, 0, i2s1, 1); // 1 = right
// GUItool: end automatically generated code

void setup() {
  AudioMemory(8);  // Allocate memory for the audio system

  // If you want to use Serial for debugging or other communication:
  // Serial.begin(9600);

  // If you're using an SD card and need to initialize it:
  // if (!SD.begin(SDCARD_CS_PIN)) {
  //   Serial.println("initialization failed!");
  //   return;
  // }
  // Serial.println("initialization done.");

  // If you want to start playing the audio immediately:
  // playMem1.play(AudioSampleBugatti);
}

void loop() {
  playMem1.play(AudioSampleBugatti);  // Replace 'Yourfile' with the actual filename
  delay(5250);
}