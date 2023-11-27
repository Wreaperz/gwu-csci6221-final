import serial
import time

# Open serial port COM4 (be sure to use the correct baud rate)
ser = serial.Serial('COM4', 9600, timeout=1)  # Adjust the baud rate as needed

# File to save the data
file_path = "audio_data.txt"
threshold = 100  # Set the threshold value to start recording
record_count = 20000  # Number of samples to record

# Start with an empty list to store the recorded samples
recorded_samples = []

iterations = 0
max_iterations = 100000

prev_line = "None"
try:
    # Wait for a value above the threshold
    while len(recorded_samples) < record_count and iterations < max_iterations:
        if len(recorded_samples) == 0:
            iterations += 1

        line = ser.readline().decode('utf-8').strip()

        if line:
            if line == "FENCE_START" or line == "FENCE_END":
                if line != prev_line:
                    recorded_samples.append(line)
            else:
                value = int(line)
                if value > threshold or len(recorded_samples) > 0:
                    recorded_samples.append(value)
                    if len(recorded_samples) == 1:
                        print("Threshold exceeded. Recording data...")
            
            # Set previous to current
            prev_line = line

    # Write the recorded samples to the file
    with open(file_path, 'w') as file:
        if len(recorded_samples) == 0:
            file.write("Max iterations reached")
        else:
            for sample in recorded_samples:
                file.write(str(sample) + '\n')

    print(f"Recording complete.")

except serial.SerialException:
    print("Could not open port COM4. Please check the connection.")
except ValueError as e:
    print(f"Value error: {e}. Check if the data received is a valid integer.")
except KeyboardInterrupt:
    print("Recording stopped by user.")
finally:
    ser.close()
    print("Serial port closed.")