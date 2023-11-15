#include <Audio.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       tone1;          //xy=233,203
AudioOutputI2S           i2s1;           //xy=447,204
AudioConnection          patchCord1(tone1, 0, i2s1, 0); // 0 = left side
AudioConnection          patchCord2(tone1, 0, i2s1, 1); // 1 = right side
AudioControlSGTL5000     sgtl5000_1;     //xy=347,338
// GUItool: end automatically generated code

void setup() {
  // Audio connections require memory to work
  AudioMemory(4);

  // Enable the audio shield
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  // Configure the waveform to be a sine wave at 1000 Hz
  tone1.begin(WAVEFORM_SINE);
  tone1.frequency(1000);
  tone1.amplitude(0.5);
}

void loop() {
  // Play the tone for 100 ms
  tone1.amplitude(0.5);
  delay(100);

  // Stop the tone
  tone1.amplitude(0);
  delay(900);
}
