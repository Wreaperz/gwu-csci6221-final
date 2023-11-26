#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Queues used to record and play data
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

AudioInputUSB            usb1; // USE THIS LINE WHEN TESTING AUDIO VIA THE PC (MicroUSB)
AudioInputI2S            line_in; // USE THIS LINE WHEN TESTING AUDIO VIA THE Condenser Mic
AudioOutputI2S           i2s2Play;

// Audio connections
AudioConnection          patchCord1(line_in, 0, queueRecord, 0); // Change "line_in" to "usb1" to use PC audio, vs Condenser Mic
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
}

void buffer_to_i2s()
{
  // UNCOMMENT THIS BLOCK TO REVERSE BITS AT A TIME
  // Reverse bits in place in the buffer: LOUD AF.
  // reverseBitsInBuffer(buffer, play_offset, MAX_SAMPLES);

  // UNCOMMENT THIS BLOCK TO REVERSE BLOCKS OF AUDIO AT A TIME - still understandable audio, but functional demo
  // Reverse the order of samples within the block in the buffer
  reverseBlockInBuffer(buffer, play_offset);

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

void loop()
{
	if(queueRecord.available() >= 2)
	{
		i2s_to_buffer(); // buffer samples

		if (mymillis > 50) // Decrease this number to make the audio feedback more "real time"
		{
			buffer_to_i2s(); //execute last buffered sample
		}
	}
}
