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

int16_t 		audio_buffer[MAX_SAMPLES];
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

void transmit_fencepost() {
  // Transmit a beep
  memcpy(queuePlay.getBuffer(), beep_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();

  // Transmit silence
  memcpy(queuePlay.getBuffer(), silence_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

// Buffer interactions
// Send a single audio packet from input queue to audio buffer
void i2s_to_audio_buffer()
{
	memcpy(audio_buffer, queueRecord.readBuffer(), MAX_SAMPLES * 2);
	queueRecord.freeBuffer();
}

// Send a single audio packet from specified buffer to output queue and play audio packet
void audio_buffer_to_i2s()
{
  memcpy(queuePlay.getBuffer(), audio_buffer, MAX_SAMPLES * 2);
  queuePlay.playBuffer();
}

void loop()
{
  // Transmit everything in one loop
	if (queueRecord.available() >= 10)
	{
    // Transmit the fencepost
    transmit_fencepost();

    int packet_count = 0;
    while (packet_count < AUDIO_PACKET_NUM) {
      if (queueRecord.available() > 0) {
        i2s_to_audio_buffer();
        audio_buffer_to_i2s();
        packet_count++;
      }
    }
	}
}
