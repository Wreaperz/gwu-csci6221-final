/* Microphone Test (https://www.youtube.com/watch?v=wqt55OAabVs&t=993s)
 * Forum thread with connection details and other info:
 * https://forum.pjrc.com/threads/60599?p=238070&viewfull=1#post238070
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=180,111
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(i2s2, 0, i2s1, 0);
AudioConnection          patchCord2(i2s2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;
// GUItool: end automatically generated code

void setup() {
  Serial.begin(9600);
  AudioMemory(8);        // amplify sign to useful range
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(40);
  delay(1000);
}

void loop() {
  // do nothing
}
