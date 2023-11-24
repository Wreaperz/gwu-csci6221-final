#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputUSB            usb1;           //xy=200,69  (must set Tools > USB Type to Audio)
AudioOutputI2S           i2s1;           //xy=365,94

// Testing queues
AudioRecordQueue         queue_recording_left;  // Left audio queue for recording
AudioRecordQueue         queue_recording_right; // Right audio queue for recording

AudioConnection          patchCord1(usb1, 0, i2s1, 0);
AudioConnection          patchCord2(usb1, 1, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;     //xy=302,184

void setup() {
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  // New loop: 

  // Test if right audio received in queue
  if (queue_recording_right.available() > 0) {
    queue_recording_right.freeBuffer(); // Free the buffer after checking

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);  
  }
}

// Teensyduino 1.35 & earlier had a problem with USB audio on Macintosh
// computers.  For more info and a workaround:
// https://forum.pjrc.com/threads/34855-Distorted-audio-when-using-USB-input-on-Teensy-3-1?p=110392&viewfull=1#post110392

