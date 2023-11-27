// EXTREMELY basic sketch, used to baseline the output capabilities of the Teensy 4.0 Microcontroller
//    - Takes input from USB and plays it back out through the line-out pins or the 3.5mm audio jack on the Audio Shield

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputUSB            usb1;           //xy=200,69  (must set Tools > USB Type to include "Audio")
AudioOutputI2S           i2s1;           //xy=365,94

AudioConnection          patchCord1(usb1, 0, i2s1, 0);
AudioConnection          patchCord2(usb1, 1, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;     //xy=302,184

void setup() {
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);
}

void loop() {
}