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


#define 		MAX_SAMPLES 	              128
#define 		MAX_QUEUE_SIZE              200
#define     DRASTIC_THRESHOLD           10000
#define     WINDOW_SIZE                 14
// Assuming 1 128 sample beep, 1 128 sample silence
#define     FENCE_WIDTH                 384
#define     AUDIO_PACKET_NUM            10
#define     PRECEDING_SILENCE           256
#define     BLOCKS_BEFORE_NORMAL_AUDIO  5


int16_t     scanning_buffer[MAX_SAMPLES];
int16_t     processing_buffer[MAX_SAMPLES * AUDIO_PACKET_NUM];

// Current length of processing buffer
int processing_buffer_max_len = MAX_SAMPLES * AUDIO_PACKET_NUM;
int processing_buffer_len = 0;

// Three Modes. One primary, two submodes if primary is false.

// Currently searching for fence start in expected area, or found fence start and traversing to fence end. This is the default state that will transmit non decrypted audio if no fence post is found.
bool processing_fence = true;  // If false, we are one of other two modes

// Distance from previous increase to current location
int prev_fence_start_dist = -100;

// If processing_fence is false
// Currently processing and sending encrypted audio
bool processing_audio_buffer = false; // If false, we have just processed and transmitted decrypted audio, and need to traverse prepended silence to expected location of next fence start.
// Amount of prepended silence that has been traversed.
int silence_traversal_progress = 0;

bool fence_start_found = false;
// Count of how many blocks have been traversed without finding a fence. Threshold of 5 blocks before normal audio is played back.
int blocks_traversed_with_no_fence = 0;
int no_fence_threshold = 5;

// Lowest expected number of samples between fences: Fence -> {(silence)(silence)(beep)(silence)(silence)}
int LOWEST_POSSIBLE_SAMPLES_BETWEEN_FENCES = FENCE_WIDTH + processing_buffer_max_len + PRECEDING_SILENCE;


// DEBUGGING
bool initialize_error_counter = false;


// Decryption algos
void reverse_audio_blocks_in_processing_buffer() 
{
  for (int packet_num = 0; packet_num < AUDIO_PACKET_NUM; packet_num++) {
    int start_ind = packet_num * MAX_SAMPLES;
    int end_ind = start_ind + MAX_SAMPLES;
    int halfway = start_ind + (MAX_SAMPLES / 2);

    // Reverse samples in packet
    for (int i = start_ind; i < halfway; i++) {
      int swap_index = end_ind - 1 - (i - start_ind);
      // Swap samples
      int16_t tmp = processing_buffer[i];
      processing_buffer[i] = processing_buffer[swap_index];
      processing_buffer[swap_index] = tmp;
    }
  }
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
  // DEBUGGING
  // if (initialize_error_counter) {
  //   Serial.print("New loop: Prev dist is: ");
  //   Serial.println(prev_fence_start_dist);
  // }
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

        // DEBUGGING
        // if (prev_fence_start_dist > 0)
        // {
        //   Serial.print("New fence found: Distance to last fence ");
        //   Serial.println(prev_fence_start_dist);
        // }

        // Serial.println(prev_fence_start_dist);
        // Serial.println("Fence start found");

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
  if (prev_fence_start_dist >= 0 && !fence_start_found) {
    prev_fence_start_dist += WINDOW_SIZE;
  }

  // If nothing is found: Return -1
  return fence_start_index;
}

void audio_buffer_to_output()
{
  // Copy and transmit all audio blocks from processing buffer to the output queue
  for (int i = 0; i < AUDIO_PACKET_NUM; i++) {
    // Serial.print("Transmitting audio packet starting from index: ");
    // Serial.println(i * MAX_SAMPLES);
    memcpy(queuePlay.getBuffer(), &processing_buffer[i * MAX_SAMPLES], MAX_SAMPLES * 2);
    queuePlay.playBuffer();
  }
}

void scanning_buffer_to_output()
{
  // Transmit single audio block from scanning buffer to output queue
  memcpy(queuePlay.getBuffer(), scanning_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

void setup()
{
  Serial.begin(9600);

  AudioMemory(12);

  queueRecord.begin();
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
          // Serial.println("Found fence start");
          // Reset prev_fence_start_dist to remaining samples in buffer to simulate further iteration
          prev_fence_start_dist = MAX_SAMPLES - fence_start_index;
          // Serial.print("Fence start dist reset to: ");
          // Serial.println(prev_fence_start_dist);

          // Serial.print("Fence previous start distance is first set to: ");
          // Serial.println(prev_fence_start_dist);
          // Reset blocks traversed with no fence to 0
          blocks_traversed_with_no_fence = 0;
        }
        // We have not found a fence in the current block.
        else 
        {
          // DEBUGGING
          // Serial.print("Blocks traversed with no fence ");
          // Serial.println(blocks_traversed_with_no_fence);

          blocks_traversed_with_no_fence++;
          // Play it as normal audio if we traverse 5 or more blocks without finding a fence
          if (blocks_traversed_with_no_fence >= BLOCKS_BEFORE_NORMAL_AUDIO) 
          {
            // Serial.println(blocks_traversed_with_no_fence);
            scanning_buffer_to_output();
          }
        }
      }
      // We have found the fence start and are traversing until the end.
      else {
        // Check traversal distance to fence end
        int traversal_distance = FENCE_WIDTH - prev_fence_start_dist;

        // If traversal distance is too great. Skip processing any values
        if (traversal_distance > MAX_SAMPLES) {
          int16_t* input_queue_pointer = queueRecord.readBuffer();
          // Free the memory
          queueRecord.freeBuffer();
          // Add the distance traversed
          prev_fence_start_dist += MAX_SAMPLES;
          // Serial.print("Update: Traversing til fence end, skipping full buffer: ");
          // Serial.println(prev_fence_start_dist);
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

          // Update prev_fence_start_dist
          prev_fence_start_dist += traversal_distance;
          // Serial.print("Update: Found fence end. Partial block loaded into processing buffer update: ");
          // Serial.println(prev_fence_start_dist);

          // Switch modes
          processing_audio_buffer = true;
        }
      }
    }
  }
  else 
  {
    // One of other two modes
    if (processing_audio_buffer)
    {
      // Process the remaining data. 
      // We will need to add somewhere between 9 and 10 packets worth of data to the processing buffer.
      if (queueRecord.available() >= AUDIO_PACKET_NUM) 
      {
        // Serial.println("Processing audio");
        // Calculate samples to copy over
        int remaining_samples = processing_buffer_max_len - processing_buffer_len;

        // Update the prev_fence_start_dist
        prev_fence_start_dist += processing_buffer_max_len;
        // Serial.print("Update: traversing processing buffer: ");
        // Serial.println(prev_fence_start_dist);
        // Calculate number of full packets that can fit into processing buffer
        int full_packets_to_insert = remaining_samples / MAX_SAMPLES;
        // Copy over the full packets
        for (int i = 0; i < full_packets_to_insert; i++) {
          memcpy(processing_buffer + processing_buffer_len, queueRecord.readBuffer(), MAX_SAMPLES * 2);
          queueRecord.freeBuffer();
          processing_buffer_len += MAX_SAMPLES;
        }
        // Serial.println("Done copying");
        // One partial packet of data to copy into processing_buffer
        remaining_samples = processing_buffer_max_len - processing_buffer_len;
        // Copy full packet into scanning buffer as intermediary
        i2s_to_scanning_buffer();
        // Serial.println("Copied into scanning buffer");
        // Copy up until last remaining sample into processing buffer. Processing buffer should now be full.
        memcpy(processing_buffer + processing_buffer_len, scanning_buffer, remaining_samples * 2);
        // Update the processing buffer length
        processing_buffer_len += remaining_samples;
        // At this point the buffer should be full. If not loop an error message.
        if (processing_buffer_len != processing_buffer_max_len) {
          Serial.println("THE PROCESSING BUFFER IS NOT PROPERLY FILLED. FIX YOUR CODE.");
        }
        // Serial.println("About to transmit audio");
        // Decrypt audio
        // reverse_audio_blocks_in_processing_buffer();

        // Transmit processed audio
        audio_buffer_to_output();
        // Serial.println("Transmitted audio");


        // RESET THIS SHIT FFS
        processing_buffer_len = 0;
        
        // Switch modes to traverse to next expected fence
        processing_audio_buffer = false;

        // Serial.println("Switching modes");

        // Calculate amount of silence already traversed by skipping partial buffer
        int partial_buffer_len = MAX_SAMPLES - remaining_samples;
        silence_traversal_progress = partial_buffer_len;

        // Update prev_fence_start_dist
        prev_fence_start_dist += silence_traversal_progress;

        // Serial.print("Switching to expect next fence: Silence traversal progress is ");
        // Serial.println(silence_traversal_progress);
      }
    }
    // Otherwise we are in prepended silence traversal mode
    else
    {
      // Serial.println("Traversing to new fence");
      if (queueRecord.available() >= 2) 
      {
        // Stop one buffer early to ensure you start scanning well before the fence occurs
        int remaining_traversal = LOWEST_POSSIBLE_SAMPLES_BETWEEN_FENCES - prev_fence_start_dist - MAX_SAMPLES;

        // HARD DEBUG WITH STOP
        // Serial.print("Remaining traversal is: ");
        // Serial.println(remaining_traversal);
        // Serial.print("Silence Traversal progress is: ");
        // Serial.println(silence_traversal_progress);
        // Serial.print("Distance to previous fence is: ");
        // Serial.println(prev_fence_start_dist);
        // delay(10000);

        // TODO:
        // Serial.println("We are here");
        // Serial.println(remaining_traversal);
        // Skip the audio packet if the remaining traversal is too large
        if (remaining_traversal > MAX_SAMPLES) 
        {
          // Serial.println(silence_traversal_progress);
          int16_t* input_queue_pointer = queueRecord.readBuffer();
          queueRecord.freeBuffer();

          // Update prev_fence_start_dist
          prev_fence_start_dist += MAX_SAMPLES;

        }
        // We know its in the next one or the one after that. However, it could be straddling the edge between two audio packets. -- Potential solution: So load two full audio packets into processing buffer.
        else 
        {
          // Switch modes and start scanning for fence again.
          // Serial.println("Scanning for new fence");
          processing_fence = true;

          fence_start_found = false;

          // initialize_error_counter = true;
          // Serial.print("Current prev fence distance is: ");
          // Serial.println(prev_fence_start_dist);
        }
      } 
    }
  }
}
