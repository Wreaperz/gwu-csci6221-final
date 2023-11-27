#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Arduino.h> // For randomSeed and random functions

// Queues used to record and play data
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

AudioInputUSB            usb1; // USE THIS LINE WHEN TESTING AUDIO VIA THE PC (MicroUSB)
AudioInputI2S            line_in; // USE THIS LINE WHEN TESTING AUDIO VIA THE Condenser Mic
AudioOutputI2S           i2s2Play;

// Audio connections
AudioConnection          patchCord1(usb1, 0, queueRecord, 0); // Change "line_in" to "usb1" to use PC audio, vs Condenser Mic
AudioConnection          patchCord2(queuePlay,  0, i2s2Play, 0); // Left Audio Queue 
AudioConnection          patchCord3(queuePlay,  0, i2s2Play, 1); // Right Audio Queue 

// Master Audio Controller
AudioControlSGTL5000     sgtl5000_1;

#define 		MAX_SAMPLES 	128
#define 		MAX_QUEUE_SIZE  200

int16_t 		buffer[MAX_SAMPLES * MAX_QUEUE_SIZE];
int32_t			record_offset = 0;
int32_t  		play_offset = 0;

elapsedMillis 	mymillis;

void setup()
{
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  AudioMemory(20);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(40);

  queueRecord.begin();

  mymillis = 0;
}

void i2s_to_buffer()
{
	memcpy(buffer + record_offset, queueRecord.readBuffer(), MAX_SAMPLES * 2);
	queueRecord.freeBuffer();
	record_offset+=MAX_SAMPLES;
	if (record_offset >= (MAX_SAMPLES * MAX_QUEUE_SIZE))	record_offset=0;
  //printBufferContents("Recorded Buffer", buffer, record_offset - MAX_SAMPLES, MAX_SAMPLES); // Debugging
}

void buffer_to_i2s()
{
  unsigned long scrambleKey = 123456789; // Example key, should be the same on both transmitter and receiver

  // UNCOMMENT THIS BLOCK TO REVERSE BITS AT A TIME
  // Reverse bits in place in the buffer: LOUD AF.
  // reverseBitsInBuffer(buffer, play_offset, MAX_SAMPLES);

  // UNCOMMENT THIS BLOCK TO REVERSE BLOCKS OF AUDIO AT A TIME - still understandable audio, but functional demo
  // Reverse the order of samples within the block in the buffer
  //reverseBlockInBuffer(buffer, play_offset);

  // Print buffer before scrambling
  printBufferContents(" Pre-Scramble", buffer, play_offset, MAX_SAMPLES);

  scrambleBlockInBuffer(buffer, play_offset, scrambleKey);
  descrambleBlockInBuffer(buffer, play_offset, scrambleKey);
  
  // Print buffer after scrambling
  printBufferContents("Post-Scramble", buffer, play_offset, MAX_SAMPLES);

  // Copy the entire block to the play queue
  memcpy(queuePlay.getBuffer(), buffer + play_offset, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Increment play_offset, wrapping around if necessary
  play_offset+=MAX_SAMPLES;
  if (play_offset >= (MAX_SAMPLES * MAX_QUEUE_SIZE)) play_offset=0;
}

// Reverse the audio samples within a block
void reverseBlockInBuffer(int16_t* buffer, int startIndex) {
    for (int i = 0; i < MAX_SAMPLES / 2; i++) {
        int swapIndex = MAX_SAMPLES - 1 - i;
        int16_t temp = buffer[startIndex + i];
        buffer[startIndex + i] = buffer[startIndex + swapIndex];
        buffer[startIndex + swapIndex] = temp;
    }
}

void scrambleBlockInBuffer(int16_t* buffer, int startIndex, unsigned long key) {
    randomSeed(key); // Initialize the random sequence with a key
    int indices[MAX_SAMPLES];
    for (int i = 0; i < MAX_SAMPLES; ++i) {
        indices[i] = i;
    }

    // Fisher-Yates shuffle algorithm
    for (int i = MAX_SAMPLES - 1; i > 0; --i) {
        int j = random(i + 1);
        int16_t temp = buffer[startIndex + indices[j]];
        buffer[startIndex + indices[j]] = buffer[startIndex + indices[i]];
        buffer[startIndex + indices[i]] = temp;
    }
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

// Reverse the bits of a chunk of data within a buffer: PROBABLY DONT DO THIS
void reverseBitsInBuffer(int16_t* buffer, int startIndex, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
      buffer[startIndex + i] = reverseBits(buffer[startIndex + i]);
    }
}

// Reverse the bits of an audio sample: PROBABLY DONT DO THIS
int16_t reverseBits(int16_t num) {
    unsigned int NO_OF_BITS = 16;
    int16_t reverse_num = 0;
    int i;
    for (i = 0; i < 16; i++) {
      if ((num & (1 << i)))
        reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
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

void loop()
{
	if(queueRecord.available() >= 2)
	{
		i2s_to_buffer(); // buffer samples
    buffer_to_i2s();
	}
}
