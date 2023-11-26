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
#define     FENCE_WIDTH        395

int16_t     scanning_buffer[MAX_SAMPLES];

// Distance from previous increase to current location
int prev_increase_dist = -100;
// Distance from current location to fence end
int fence_end_dist = -100;



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


void scan_buffer_for_start_of_fence(int16_t* buffer, int* fence_start_and_end) 
{
  // Sliding window.
  for (int i = 0; i < MAX_SAMPLES - WINDOW_SIZE; i++) {

    // Distance calculations
    // Increment distance between current location and prev_increase
    if (prev_increase_dist >= 0) {
      prev_increase_dist++;
    }
    // Decrement distance between current location and fence_end.
    if (fence_end_dist > 0) {
      fence_end_dist--;
    }
    // Check if arrived at fence end
    if (fence_end_dist == 0) {
      fence_start_and_end[1] = i;
      // Initialize fence_end_dist to nonsensical value until new fence start is found
      fence_end_dist = -1000;
    }

    // Calculate window boundaries
    int window_start = i;
    int window_end = i + WINDOW_SIZE;

    // Process window if within buffer
    int diff = buffer[window_end] - buffer[window_start];

    if (diff >= DRASTIC_THRESHOLD) {
      // Check distance between this find and previous find
      if (prev_increase_dist > (FENCE_WIDTH + MAX_SAMPLES) || prev_increase_dist < 0) {
        int midpoint = window_start + (WINDOW_SIZE / 2);
        fence_start_and_end[0] = midpoint;
        // Set current window_start to location of prev_increase
        prev_increase_dist = 0;
        // Set distance to fence end
        fence_end_dist = (WINDOW_SIZE / 2) + FENCE_WIDTH;
      }
    }
  }

  // Iterate remaining values 
  for (int i = MAX_SAMPLES - WINDOW_SIZE; i < MAX_SAMPLES; i++) {
    // Distance calculations
    // Increment distance between current location and prev_increase
    if (prev_increase_dist >= 0) {
      prev_increase_dist++;
    }
    // Decrement distance between current location and fence_end.
    if (fence_end_dist > 0) {
      fence_end_dist--;
    }
    // Check if arrived at fence end
    if (fence_end_dist == 0) {
      fence_start_and_end[1] = i;
      // Initialize fence_end_dist to nonsensical value until new fence start is found
      fence_end_dist = -1000;
    }
  }
}

void buffer_to_i2s()
{
  // Initialize fence_start_and_end to nonsensical values
  int fence_start_and_end[2];
  fence_start_and_end[0] = -100;
  fence_start_and_end[1] = -100;

  // Scan buffer for start or end of fence
  scan_buffer_for_start_of_fence(scanning_buffer, fence_start_and_end);

  // Send audio block to PC with increase markers
  sendAudioBlockToPC(scanning_buffer, fence_start_and_end);

  // Copy the entire block to the play queue
  memcpy(queuePlay.getBuffer(), scanning_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

void sendAudioBlockToPC(int16_t* buffer, int* fence_start_and_end) {
  // If not found, fence start and end are negative.

  for (int i = 0; i < MAX_SAMPLES; i++) {
    // Check if fence_start is current index
    if (fence_start_and_end[0] == i) {
      Serial.println("FENCE_START");
    }

    // Check if fence_end is current index
    if (fence_start_and_end[1] == i) {
      Serial.println("FENCE_END");
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
