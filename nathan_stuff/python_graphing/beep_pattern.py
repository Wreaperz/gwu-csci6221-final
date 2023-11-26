import numpy as np
import matplotlib.pyplot as plt

# Constants
PI = np.pi

def initializeBeepBuffer(bufferSize, frequency=1000, sampleRate=44100, envelopeLength=20):
    # Initialize the buffer
    buffer = np.zeros(bufferSize, dtype=np.int16)
    
    phaseIncrement = 2.0 * PI * frequency / sampleRate
    phase = 0.0

    for i in range(bufferSize):
        envelope = 1.0  # No envelope by default
        # Apply envelope at the beginning
        if i < envelopeLength:
            envelope = float(i) / envelopeLength
        # Apply envelope at the end
        if i >= bufferSize - envelopeLength:
            envelope = float(bufferSize - i) / envelopeLength
        buffer[i] = int(32767.0 * np.sin(phase) * envelope)
        phase += phaseIncrement
        if phase > 2.0 * PI:
            phase -= 2.0 * PI  # Keep the phase within the range [0, 2*PI]

    return buffer

# Initialize a buffer of 128 integers with a beep sound
bufferSize = 128
beepBuffer = initializeBeepBuffer(bufferSize)

# Print the buffer values
print(beepBuffer)

# Plot the buffer
plt.plot(beepBuffer)
plt.title('Beep Sound Waveform')
plt.xlabel('Sample Number')
plt.ylabel('Amplitude')
plt.savefig('beep_pattern_in_python.png')