#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Arduino.h> // For randomSeed and random functions

// Audio components
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;
AudioInputI2S            line_in; // Line-in input
AudioOutputI2S           i2s2Play;

// Define the low-pass filter
AudioFilterBiquad        lowpassFilter;

// Audio connections
AudioConnection          patchCord1(line_in, 0, queueRecord, 0); // Line-in connection
AudioConnection          patchCord2(queuePlay, 0, lowpassFilter, 0); // Connect queuePlay to lowpassFilter
AudioConnection          patchCord3(lowpassFilter, 0, i2s2Play, 0); // Connect lowpassFilter to i2s2Play (Left)
AudioConnection          patchCord4(lowpassFilter, 0, i2s2Play, 1); // Connect lowpassFilter to i2s2Play (Right)

// Master Audio Controller
AudioControlSGTL5000     sgtl5000_1;

#define MAX_SAMPLES     128
#define MAX_QUEUE_SIZE  200

int16_t buffer[MAX_SAMPLES * MAX_QUEUE_SIZE];
int32_t record_offset = 0;
int32_t play_offset = 0;

int16_t globalMin = INT16_MAX; // Initialize to the maximum possible value of int16_t
int16_t globalMax = INT16_MIN; // Initialize to the minimum possible value of int16_t


elapsedMillis mymillis;

void setup() {

  Serial.begin(115200); // Initialize serial communication at 115200 baud rate

  AudioMemory(20);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  // sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); // Select line-in as the audio source
  // sgtl5000_1.micGain(40);

  queueRecord.begin();
  lowpassFilter.setLowpass(0, 2500); // Example cutoff frequency at 1500 Hz
  mymillis = 0;
}

void line_in_to_buffer() {
  memcpy(buffer + record_offset, queueRecord.readBuffer(), MAX_SAMPLES * 2);
  queueRecord.freeBuffer();
  record_offset += MAX_SAMPLES;
  if (record_offset >= (MAX_SAMPLES * MAX_QUEUE_SIZE)) record_offset = 0;
}

void buffer_to_i2s() {
  unsigned long scrambleKey = 123456789; // Use the same key as in the transmitter

  // Print buffer before descrambling
  printBufferContents(" Pre-Descramble", buffer, play_offset, MAX_SAMPLES);

  // Descramble the audio data
  //descrambleBlockInBuffer(buffer, play_offset, scrambleKey);

  // Print buffer after descrambling
  printBufferContents("Post-Descramble", buffer, play_offset, MAX_SAMPLES);

  // Play the descrambled audio
  memcpy(queuePlay.getBuffer(), buffer + play_offset, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
  
  play_offset += MAX_SAMPLES;
  if (play_offset >= (MAX_SAMPLES * MAX_QUEUE_SIZE)) play_offset = 0;
}

void descrambleBlockInBuffer(int16_t* buffer, int startIndex, unsigned long key) {
    randomSeed(key); // Initialize the random sequence with the same key
    int indices[MAX_SAMPLES];
    for (int i = 0; i < MAX_SAMPLES; ++i) {
        indices[i] = i;
    }

    // Generate the same shuffle pattern
    int swapIndices[MAX_SAMPLES];
    for (int i = MAX_SAMPLES - 1; i > 0; --i) {
        int j = random(i + 1);
        swapIndices[i] = j;
    }

    // Reverse the Fisher-Yates shuffle
    for (int i = 1; i < MAX_SAMPLES; ++i) {
        int j = swapIndices[i];
        int16_t temp = buffer[startIndex + indices[j]];
        buffer[startIndex + indices[j]] = buffer[startIndex + indices[i]];
        buffer[startIndex + indices[i]] = temp;
    }
}

void printBufferContents(const char* label, int16_t* buffer, int startIndex, int count) {
    Serial.print(label);
    Serial.print(": ");
    for (int i = 0; i < count; ++i) {
        Serial.print(buffer[startIndex + i]);
        Serial.print(" ");
    }
    Serial.println();
}

void loop() {
  if (queueRecord.available() >= 2) {
    line_in_to_buffer(); // Buffer samples from line-in

    if (mymillis > 10) { // Adjust this for real-time feedback
      buffer_to_i2s(); // Play the last buffered sample
    }
  }
}
