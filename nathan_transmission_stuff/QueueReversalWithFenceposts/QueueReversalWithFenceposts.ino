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
AudioControlSGTL5000     sgtl5000_1;


#define 		MAX_SAMPLES 	     128
#define 		MAX_QUEUE_SIZE     200
#define     AUDIO_PACKET_NUM   10

int16_t 		audio_buffer[MAX_SAMPLES * AUDIO_PACKET_NUM];
int16_t     beep_buffer[MAX_SAMPLES];
int16_t     silence_buffer[MAX_SAMPLES];

void setup()
{
  AudioMemory(20);

  queueRecord.begin();

  // Initialize the beep buffer
  initializeBeepBuffer(beep_buffer, MAX_SAMPLES);

  // Initialize the silence buffer
  memset(silence_buffer, 0, MAX_SAMPLES * sizeof(int16_t));
}

void initializeBeepBuffer(int16_t* buffer, int bufferSize) {

  int frequency = 1000;
  int sampleRate = 44100;
  int envelopeLength = 20;

  float phaseIncrement = 2.0 * PI * frequency / sampleRate;
  float phase = 0.0;

  for (int i = 0; i < bufferSize; ++i) {
    float envelope = 1.0; // No envelope by default
    // Apply envelope at the beginning
    if (i < envelopeLength) {
      envelope = (float)i / envelopeLength;
    }
    // Apply envelope at the end
    if (i >= bufferSize - envelopeLength) {
      envelope = (float)(bufferSize - i) / envelopeLength;
    }
    buffer[i] = (int16_t)(32767.0 * sin(phase) * envelope);
    phase += phaseIncrement;
    if (phase > 2.0 * PI) {
      phase -= 2.0 * PI; // Keep the phase within the range [0, 2*PI]
    }
  }
}


// Transmit the fenceposts
void transmit_fencepost() {
  // Transmit silence
  memcpy(queuePlay.getBuffer(), silence_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Transmit silence
  memcpy(queuePlay.getBuffer(), silence_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Transmit a beep
  memcpy(queuePlay.getBuffer(), beep_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Transmit silence
  memcpy(queuePlay.getBuffer(), silence_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Transmit silence
  memcpy(queuePlay.getBuffer(), silence_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

// Sample reversal encryption
void reverse_samples_in_audio_buffer() 
{
  int buffer_size = MAX_SAMPLES * AUDIO_PACKET_NUM;
  int halfway = buffer_size / 2;

  for (int i = 0; i < halfway; i++) {
    // Calculate the index of the element to swap with
    int swap_index = buffer_size - 1 - i;

    // Swap the elements
    int16_t tmp = audio_buffer[i];
    audio_buffer[i] = audio_buffer[swap_index];
    audio_buffer[swap_index] = tmp;
  }
}

// Sample reversal encryption
void reverse_audio_blocks_in_audio_buffer() 
{
  for (int packet_num = 0; packet_num < AUDIO_PACKET_NUM; packet_num++) {
    int start_ind = packet_num * MAX_SAMPLES;
    int end_ind = start_ind + MAX_SAMPLES;
    int halfway = start_ind + (MAX_SAMPLES / 2);

    // Reverse samples in packet
    for (int i = start_ind; i < halfway; i++) {
      int swap_index = end_ind - 1 - (i - start_ind);
      // Swap samples
      int16_t tmp = audio_buffer[i];
      audio_buffer[i] = audio_buffer[swap_index];
      audio_buffer[swap_index] = tmp;
    }
  }
}

// Buffer interactions
// Send a single audio packet from input queue to audio buffer
void i2s_to_audio_buffer()
{
  for (int i = 0; i < AUDIO_PACKET_NUM; i++) {
    memcpy(&audio_buffer[i * MAX_SAMPLES], queueRecord.readBuffer(), MAX_SAMPLES * 2);
    queueRecord.freeBuffer();
  }
}

// Send multiple audio packet from specified buffer to output queue and play audio packet
void buffer_to_i2s()
{
  for (int i = 0; i < AUDIO_PACKET_NUM; i++) {
    memcpy(queuePlay.getBuffer(), &audio_buffer[i * MAX_SAMPLES], MAX_SAMPLES * 2);
    queuePlay.playBuffer();
  }
}

void loop()
{
	if (queueRecord.available() >= AUDIO_PACKET_NUM)
	{

    // Transfer audio packets from input queue to audio_buffer
		i2s_to_audio_buffer(); 

    // // Process the data
    // reverse_samples_in_audio_buffer();
    // reverse_samples_in_audio_buffer();

    // reverse_audio_blocks_in_audio_buffer();
    // reverse_audio_blocks_in_audio_buffer();

    // // Transmit fenceposts
    transmit_fencepost();

    // Transmit processed audio
		buffer_to_i2s();
	}
}
