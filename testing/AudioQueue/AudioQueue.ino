#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputUSB            usb1;           //xy=200,69  (must set Tools > USB Type to Audio)
AudioOutputI2S           i2s1;           //xy=365,94

// Input audio buffers
AudioRecordQueue         queue_recording_left;         // Left audio queue takes input for processing
AudioRecordQueue         queue_recording_right;         // Right audio queue takes input for processing

// Output audio buffers, hold post processed data
AudioPlayQueue        queue_play_left;         // Left audio queue takes processed data to output
AudioPlayQueue        queue_play_right;         // Right audio queue takes processed data to output


// From input to recording queue to hold for processing
AudioConnection          patchCord1(usb1, 0, queue_recording_left, 0);
AudioConnection          patchCord2(usb1, 1, queue_recording_right, 1);

// From output queues to output after data is processed
AudioConnection          patchCord3(queue_play_left, 0, i2s1, 0);
AudioConnection          patchCord4(queue_play_right, 1, i2s1, 1);

AudioControlSGTL5000     sgtl5000_1;     //xy=302,184


#define 		MAX_SAMPLES 	128
#define     SAMPLE_SIZE   2

// Function that reverses the bits of a sample
int16_t reverseBits(int16_t sample) {
    // No operation, return the sample unchanged
    return sample;
}

void setup() {
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  queue_recording_left.begin();
  queue_recording_right.begin();
}

void process_samples() 
{
  // Get data from input queues
  if (queue_recording_left.available() > 0 && queue_recording_right.available() > 0)
  {
    // Pointers
    int16_t *input_left = queue_recording_left.readBuffer();
    int16_t *input_right = queue_recording_right.readBuffer();

    // Process left audio data (bit-reversal)
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      input_left[i] = input_left[i];
    }

    // Process right audio data
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      input_right[i] = input_right[i];
    }

    // Write processed left audio data to play queue
    int16_t *output_left = queue_play_left.getBuffer();
    memcpy(output_left, input_left, AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
    queue_play_left.playBuffer();

    // Write processed right audio to play queue
    int16_t *output_right = queue_play_right.getBuffer();
    memcpy(output_right, input_right, AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
    queue_play_right.playBuffer();

    // Release the record queue buffers
    queue_recording_left.freeBuffer();
    queue_recording_right.freeBuffer();
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void loop() {

  process_samples();

  // read the PC's volume setting
  float vol = usb1.volume();

  // scale to a nice range (not too loud)
  // and adjust the audio shield output volume
  if (vol > 0) {
    // scale 0 = 1.0 range to:
    //  0.3 = almost silent
    //  0.8 = really loud
    vol = 0.3 + vol * 0.5;
  }

  // use the scaled volume setting.  Delete this for fixed volume.
  sgtl5000_1.volume(vol);

  delay(100);
}

// Teensyduino 1.35 & earlier had a problem with USB audio on Macintosh
// computers.  For more info and a workaround:
// https://forum.pjrc.com/threads/34855-Distorted-audio-when-using-USB-input-on-Teensy-3-1?p=110392&viewfull=1#post110392

