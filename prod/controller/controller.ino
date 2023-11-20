#include <Audio.h>

// GUItool: begin automatically generated code
AudioInputI2S            jack_input;           //xy=313,157
AudioPlayQueue           jack_queue_play;      //xy=500,240
AudioRecordQueue         jack_queue_record;    //xy=501,203
AudioOutputI2S           jack_output;          //xy=709,313
AudioSynthWaveform       tone1;                //xy=233,203
AudioConnection          patchCord1(jack_input, 0, jack_queue_record, 0);
AudioConnection          patchCord2(jack_input, 1, jack_queue_record, 0);
AudioConnection          patchCord3(jack_queue_play, 0, jack_output, 0);
AudioConnection          patchCord4(jack_queue_play, 0, jack_output, 1);
AudioConnection          patchCord5(tone1, 0, jack_output, 0); // Connecting beep to output
AudioConnection          patchCord6(tone1, 0, jack_output, 1);
AudioControlSGTL5000     sgtl5000_1;           //xy=313,108
// GUItool: end automatically generated code

void beep() {
  tone1.amplitude(0.5);
  delay(100);  // Beep for 100 ms
  tone1.amplitude(0);
}

void setup() {
  AudioMemory(60); 

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  tone1.begin(WAVEFORM_SINE);
  tone1.frequency(1000);
  tone1.amplitude(0);
}

void loop() {
  delay(2000); // Pause for 2 seconds
  beep(); // Play beep sound

  jack_queue_record.begin(); // Start recording
  delay(5000); // Record for 5 seconds

  beep(); // Play another beep sound

  jack_queue_record.end(); // Stop recording
  delay(1000); // Pause for 1 second

  // Playback what's in the queue
  while (jack_queue_record.available()) {
    int16_t *buffer;
    buffer = jack_queue_record.readBuffer();
    jack_queue_play.playBuffer();
    jack_queue_record.freeBuffer();
  }

  delay(5000); // Delay before repeating the process
}
