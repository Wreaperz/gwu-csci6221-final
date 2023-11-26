#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


// Queues
AudioRecordQueue         queueRecord;
AudioPlayQueue           queuePlay;

// USED WHEN SENDING DATA
//AudioInputUSB            usb_input;

// USED WHEN RECEIVING DATA
AudioInputI2S            line_input;           //xy=460,210

AudioOutputI2S           i2s2Play;
AudioConnection          patchCord1(line_input, 0, queueRecord, 0);
AudioConnection          patchCord2(queuePlay,  0, i2s2Play, 0);
//AudioConnection          patchCord3(queuePlay,  0, i2s2Play, 1);
AudioControlSGTL5000     sgtl5000_1;


#define 		MAX_SAMPLES 	     128
#define 		MAX_QUEUE_SIZE     200
#define     DRASTIC_THRESHOLD  10000
#define     WINDOW_SIZE        14

int16_t     scanning_buffer[MAX_SAMPLES];

// Layover value, represents the last value of the previous audio buffer
int16_t     previous_value = 10;

void setup()
{
  Serial.begin(9600);

  AudioMemory(12);

  queueRecord.begin();
}

// Transfers audio block from input queue to scanning buffer
void i2s_to_buffer()
{
	memcpy(scanning_buffer, queueRecord.readBuffer(), MAX_SAMPLES * 2);
	queueRecord.freeBuffer();
}

int scan_buffer_for_increases(int16_t* buffer, int* increase_index_list) 
{
  // Placeholder for previous increase
  int prev_increase = -100;

  // Length of increase index list
  int increase_index_list_len = 0;

  // Sliding window
  for (int i = 0; i < MAX_SAMPLES - WINDOW_SIZE; i++) {
    int window_start = i;
    int window_end = i + WINDOW_SIZE;

    int diff = buffer[window_end] - buffer[window_start];

    if (diff >= DRASTIC_THRESHOLD) {
      // Check distance between this find and previous find
      int dist = window_start - prev_increase;
      if (dist > 20) {
        int midpoint = window_start + (WINDOW_SIZE / 2);
        increase_index_list[increase_index_list_len] = midpoint;
        increase_index_list_len++;
        prev_increase = window_start;
      }
    }
  }

  return increase_index_list_len;
}

void buffer_to_i2s()
{
  // // Scan buffer for large increases 
  int increase_index_list[128];
  // int increase_list_len = 0;
  int increase_list_len = scan_buffer_for_increases(scanning_buffer, increase_index_list);

  // // Send audio block to PC with increase markers
  sendAudioBlockToPC(scanning_buffer, increase_index_list, increase_list_len);

  // Copy the entire block to the play queue
  memcpy(queuePlay.getBuffer(), scanning_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

void sendAudioBlockToPC(int16_t* buffer, int* increase_index_list, int increase_index_list_len) {
    for (int i = 0; i < MAX_SAMPLES; i++) {

      // Check if the current index is in the increase_index_list
      bool isDrasticIncrease = false;
      for (int j = 0; j < increase_index_list_len; j++) {
        if (i == increase_index_list[j]) {
          isDrasticIncrease = true;
          // Remove the value from the array
          increase_index_list[j] = increase_index_list[increase_index_list_len - 1];
          increase_index_list_len--;
          break;  // Break the loop once a match is found
        }
      }

      // If the current index is a point of drastic increase, print the marker
      if (isDrasticIncrease) {
        Serial.println("DRASTIC_INCREASE");
      }

      // Print the digital audio value
      Serial.println(buffer[i]);
    }
}

void loop()
{
	if(queueRecord.available() >= 2)
	{
		i2s_to_buffer(); // buffer samples
		buffer_to_i2s(); //execute last buffered sample
  }
}
