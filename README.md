# gwu-csci6221-final
Term Project for GWU's CSCI 6221

**Team Members:**

    Vinayak Dubey   (G47804140)
    Suprit Patil    (G37575767)
    Caleb Carpenter (G46027348)
    Nathan Ormsby   (G32833160)

**Project Concept:** _Encrypted Two-Way Radio_

**Project Details:**

    Project Issue: 
    
      Two-way radios are a staple form of communication in remote areas, or in situations where 
      handheld phones are too delicate (construction sites, etc.). However, these types of radios offer no privacy, 
      and allow anyone to listen in on conversations. This project aims to resolve that issue by performing live encryption 
      on the audio data being transmitted between devices (prior to transmission), 
      and then performing live decryption and playback of that audio.

    Project Parts (x2 = Quantity: 2)
      - x2 Teensy 4.0 Microcontroller (https://www.amazon.com/dp/B08259KDHY)
      - x2 Teensy 4.0 Audio Shield (https://www.amazon.com/dp/B07Z6NW913)
      - 4-pck Radio Handsets (https://www.amazon.com/dp/B08MKT9B7X)

**Testing and Resources**

    Preliminary Hardware/Software Testing
        Once you've purchased your microcontrollers and shields, you'll need to solder female headers to the audio shield,
        to be able to connect the Teensy board. After this, you'll want to test out the hardware and software capabilities
        to ensure the microcontroller and audio shield are connected properly.

        Inside the "testing" folder you'll find "AudioShieldBeepTest" and "WavFilePlayer". The former is a simply sketch
        that will ensure your hardware/software is working. Just use the Arduino IDE to upload the sketch, and then
        plug in a set of wired headphones into the jack, on the audio shield. You should hear a beep every second or so.
        WavFilePlayer is a more complex sketch that just demos the capability to play files from memory. Just put a .wav
        file in the "wav2sketch" folder, run "wav2sketch.exe", and then copy+paste the 2 created files into the WavFilePlayer
        directory. Then, you're going to upload the sketch (and ensure headphones are plugged in). This will play whatever .wav
        file that you've created (you might need to adjust the 'delay' time, as it's specifically set for the Bugatti test file).
