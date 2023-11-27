#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// Queues
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

// USED WHEN SENDING DATA
AudioInputUSB            usb_input;

// USED WHEN RECEIVING DATA
AudioInputI2S            line_input;           //xy=460,210

AudioOutputI2S           i2s2Play;
AudioConnection          patchCord1(usb_input, 0, queueRecord, 0);
AudioConnection          patchCord2(queuePlay,  0, i2s2Play, 0);
//AudioConnection          patchCord3(queuePlay,  0, i2s2Play, 1);
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

void loop()
{

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
