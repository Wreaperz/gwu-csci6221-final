// EXTREMELY basic sketch, used to baseline the output capabilities of the Teensy 4.0 Microcontroller
//    - Takes input from Condenser Microphone and plays it back out through the line-out pins or the 3.5mm audio jack on the Audio Shield

// NOTE: MUST have a 2-pin condenser mic soldered onto the Audio Shield (pins are on the opposite side of the SD card reader)

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputI2S            i2s2;           //xy=180,111
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(i2s2, 0, i2s1, 0); // Left = 0
AudioConnection          patchCord2(i2s2, 0, i2s1, 1); // Right = 1
AudioControlSGTL5000     sgtl5000_1;

void setup() {
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(40);
  delay(1000);
}

void loop() {
  // do nothing
}
