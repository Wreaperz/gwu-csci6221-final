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
int16_t     processing_buffer[MAX_SAMPLES * AUDIO_PACKET_NUM];

// Current length of processing buffer
int processing_buffer_max_len = MAX_SAMPLES * AUDIO_PACKET_NUM;
int processing_buffer_len = 0;

// Distance from previous increase to current location
int prev_fence_start_dist = -100;
// Distance from current location to fence end
int fence_end_dist = -100;

bool processing_fence = true;
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
  // If processing fence, we are either looking for fence start, or traversing start to end.
  if (processing_fence) {
    // Data must be available to do anything
    if (queueRecord.available() >= 1) 
    {
      // We are currently scanning for fence start
      if (!fence_start_found) {
        // Move data to scanning buffer
        i2s_to_scanning_buffer();
        // Scan buffer for fence_start
        int fence_start_index = scan_buffer_for_start_of_fence(scanning_buffer);
        // Check if found
        if (fence_start_found) {
          // Reset prev_fence_start_dist to remaining samples in buffer to simulate further iteration
          prev_fence_start_dist = MAX_SAMPLES - fence_start_index;
        }
      }
      // We have found the fence start and are traversing until the end.
      else {
        // Check traversal distance to fence end
        int traversal_distance = FENCE_WIDTH - prev_fence_start_dist;

        // If traversal distance is too great. Skip processing any values
        if traversal_distance > MAX_SAMPLES {
          int16_t* input_queue_pointer = queueRecord.readBuffer();
          // Free the memory
          queueRecord.freeBuffer();
          // Add the distance traversed
          prev_fence_start_dist += MAX_SAMPLES;
        }
        // Fence end is within the next audio block
        else {
          // Signal the fence traversal is complete
          processing_fence = false;

          // Transfer data to scanning buffer
          i2s_to_scanning_buffer();

          // Copy leftover samples to processing buffer
          int samples_to_copy = MAX_SAMPLES - traversal_distance;
          int start_copy_index = traversal_distance;
          memcpy(processing_buffer + processing_buffer_len, scanning_buffer + start_copy_index, samples_to_copy * sizeof(int16_t));

          // Update processing buffer length
          processing_buffer_len += samples_to_copy;
        }
      }
    }
  }
  else {
    // Process the remaining data. 
    // We will need to add somewhere between 9 and 10 packets worth of data to the processing buffer.
    if (queueRecord.available() >= AUDIO_PACKET_NUM) {
      // Calculate samples to copy over
      int remaining_samples = processing_buffer_max_len - processing_buffer_len;
      // Calculate number of full packets that can fit into processing buffer
      int full_packets_to_insert = remaining_samples / MAX_SAMPLES;
      // Copy over the full packets
      for (int i = 0; i < full_packets_to_insert; i++) {
        memcpy(processing_buffer + processing_buffer_len, queueRecord.readBuffer(), MAX_SAMPLES * 2);
        queueRecord.freeBuffer();
        processing_buffer_len += MAX_SAMPLES;
      }
      // One partial packet of data to copy into processing_buffer
      remaining_samples = processing_buffer_max_len - processing_buffer_len;
      // Copy full packet into scanning buffer as intermediary
      i2s_to_scanning_buffer();
      // Copy up until last remaining sample into processing buffer. Processing buffer should now be full.
      memcpy(processing_buffer + processing_buffer_len, scanning_buffer, remaining_samples * 2);
      // Traverse until fence end: Dont worry about for now.
    }
  }
}
