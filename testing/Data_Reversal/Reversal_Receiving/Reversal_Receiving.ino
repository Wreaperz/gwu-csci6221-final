#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// Queues
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

// AudioInputI2S            i2s1Record;
AudioInputI2S            i2s1;
AudioAnalyzePeak         peak1;

//AudioAnalyzePeak         peak2;
AudioOutputI2S           i2s2Play;
AudioConnection          patchCord1(i2s1, 0, queueRecord, 0);
AudioConnection          patchCord2(queuePlay,  0, i2s2Play, 0);
AudioConnection          patchCord3(queuePlay,  0, i2s2Play, 1);
AudioConnection          patchCord4(usb1, 0, peak1, 0);
//AudioConnection          patchCord4(i2s1, 0, peak2, 1);
// AudioControlSGTL5000     sgtl5000_1;


#define 		MAX_SAMPLES 	128
#define 		MAX_QUEUE_SIZE  200

int16_t 		buffer[MAX_SAMPLES * MAX_QUEUE_SIZE];
int32_t			record_offset = 0;
int32_t  		play_offset = 0;


elapsedMillis 	mymillis;


// INPUT     OUTPUT
// i2s1 L >> i2s2 L
// i2s1 R >> i2s2 R

void setup()
{
  AudioMemory(8);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  queueRecord.begin();

  mymillis = 0;
}

void printPeak()
{
	Serial.println(peak1.read());
	delay(5);
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
  // Reverse bits in place in the buffer: LOUD AF.
  reverseBitsInBuffer(buffer, play_offset, MAX_SAMPLES);

  // Reverse the order of samples within the block in the buffer
  // reverseBlockInBuffer(buffer, play_offset);

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

	//printPeak();
	//delay(1000);

	if(queueRecord.available() >= 2)
	{

    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(100);
    // digitalWrite(LED_BUILTIN, LOW);
    // delay(100);  

		i2s_to_buffer(); // buffer samples


		if (mymillis > 500) //if more than 100ms elapsed from last buffer play
		{
			buffer_to_i2s(); //execute last buffered sample
		}

		//Serial.print(record_offset);
		//Serial.print(" ");
		//Serial.print(play_offset);
		//Serial.println();

	}// end if(queueRecord.available() >= 2)
}
