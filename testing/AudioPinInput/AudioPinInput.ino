#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=460,210
AudioOutputI2S           i2s2;           //xy=709,216
AudioConnection          patchCord1(i2s1, 0, i2s2, 0);
AudioConnection          patchCord2(i2s1, 1, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=473,150
// GUItool: end automatically generated code

void setup() {
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {
  // do nothing
}

