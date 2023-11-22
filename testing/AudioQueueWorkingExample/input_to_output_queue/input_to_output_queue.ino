#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// Queues
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

// AudioInputI2S            i2s1Record;
AudioInputUSB            usb1;
AudioAnalyzePeak         peak1;

//AudioAnalyzePeak         peak2;
AudioOutputI2S           i2s2Play;
AudioConnection          patchCord1(usb1, 0, queueRecord, 0);
AudioConnection          patchCord2(queuePlay,  0, i2s2Play, 1);
AudioConnection          patchCord3(usb1, 0, peak1, 0);
//AudioConnection          patchCord4(i2s1, 0, peak2, 1);
// AudioControlSGTL5000     sgtl5000_1;


#define 		MAX_SAMPLES 	128
#define 		MAX_QUEUE_SIZE  200

uint16_t 		buffer[MAX_SAMPLES * MAX_QUEUE_SIZE];
int32_t			record_offset = 0;
int32_t  		play_offset = 0;


elapsedMillis 	mymillis;


// INPUT     OUTPUT
// i2s1 L >> i2s2 L
// i2s1 R >> i2s2 R

void setup()
{
  AudioMemory(8);

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
   memcpy(queuePlay.getBuffer(), buffer + play_offset , MAX_SAMPLES * 2);
    queuePlay.playBuffer();
    play_offset+=MAX_SAMPLES;
    if (play_offset >= (MAX_SAMPLES * MAX_QUEUE_SIZE)) play_offset=0;
}

input_buffer_to_output()
{
  memcpy(queuePlay.getBuffer(), queueRecord.readBuffer(), MAX_SAMPLES * 2);
  queueRecord.freeBuffer();
  queuePlay.playBuffer();
}


void loop()
{

	//printPeak();
	//delay(1000);

	if(queueRecord.available() >= 2)
	{

    // Old code
		// i2s_to_buffer(); // buffer samples


		// if (mymillis > 500) //if more than 100ms elapsed from last buffer play
		// {
		// 	buffer_to_i2s(); //execute last buffered sample
		// }

    // Direct transfer between queues
    input_buffer_to_output();

		//Serial.print(record_offset);
		//Serial.print(" ");
		//Serial.print(play_offset);
		//Serial.println();

	}// end if(queueRecord.available() >= 2)
}
