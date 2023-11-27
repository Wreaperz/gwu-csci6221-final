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
// Assuming 1 128 sample beep, 1 128 sample silence
#define     FENCE_WIDTH        384
#define     AUDIO_PACKET_NUM   10


int16_t     scanning_buffer[MAX_SAMPLES];
int16_t     processing_buffer[MAX_SAMPLES * 10];

// Distance from previous increase to current location
int prev_fence_start_dist = -100;
// Distance from current location to fence end
int fence_end_dist = -100;


bool fence_start_found = false;


void setup()
{
  Serial.begin(9600);

  AudioMemory(12);

  queueRecord.begin();
}

// Transfers audio block from input queue to scanning buffer
void i2s_to_scanning_buffer()
{
	memcpy(scanning_buffer, queueRecord.readBuffer(), MAX_SAMPLES * 2);
	queueRecord.freeBuffer();
}

// If fence is not found, look for it.
int scan_buffer_for_start_of_fence(int16_t* buffer) 
{
  // Fence_start_index
  int fence_start_index = -1;

  // Sliding window.
  for (int i = 0; i < MAX_SAMPLES - WINDOW_SIZE; i++) {

    // Distance calculations
    // Increment distance between current location and prev_fence_start
    if (prev_fence_start_dist >= 0) {
      prev_fence_start_dist++;
    }

    // Calculate window boundaries
    int window_start = i;
    int window_end = i + WINDOW_SIZE;

    // Process window
    int diff = buffer[window_end] - buffer[window_start];

    if (diff >= DRASTIC_THRESHOLD) {
      // Check distance between this find and previous find
      if (prev_fence_start_dist > (FENCE_WIDTH + 5 * MAX_SAMPLES) || prev_fence_start_dist < 0) {
        int midpoint = window_start + (WINDOW_SIZE / 2);
        // Set fence_start_found flag
        fence_start_found = true;
        // Set fence_start_index
        fence_start_index = midpoint;
        // No further iteration is needed.
        break;
      }
    }
  }

  // Add remaining samples to prev_fence_start_dist if made it this far
  if (prev_fence_start_dist >= 0) {
    prev_fence_start_dist += WINDOW_SIZE;
  }

  // If nothing is found: Return -1
  return fence_start_index;
}

void buffer_to_i2s()
{
  // Copy the entire block to the play queue
  memcpy(queuePlay.getBuffer(), scanning_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

// Send single digit to PC via serial
void sendSerialNumToPC(int num) {
  Serial.println(num);
}

void loop()
{
  // If fence_start_not_found. Need to look for fence start in scanning buffer
  if (!fence_start_found) {
    if (queueRecord.available() >= 1)
    {
      // Move data to scanning buffer
      i2s_to_scanning_buffer();
      // Scan buffer for fence_start
      int fence_start_index = scan_buffer_for_start_of_fence(scanning_buffer);
      // Check if found
      if (fence_start_found) {
        // Output distance between current fence_post_start and previous if not the first
        if (prev_fence_start_dist >= 0) {
          sendSerialNumToPC(prev_fence_start_dist);
        }
        // Reset prev_fence_start_dist to remaining samples in buffer to simulate further iteration
        prev_fence_start_dist = MAX_SAMPLES - fence_start_index;

        // Set fence_start_found to false and repeat
        fence_start_found = false;
      }
    }
  }
  else {
    Serial.println("PROGRAM AINT WORKIN");
    delay(100);
  }
}
