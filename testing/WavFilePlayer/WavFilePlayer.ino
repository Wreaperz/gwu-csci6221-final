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
}

void loop() {
  playMem1.play(AudioSampleBugatti);  // Replace 'Yourfile' with the actual filename
  delay(5250);
}