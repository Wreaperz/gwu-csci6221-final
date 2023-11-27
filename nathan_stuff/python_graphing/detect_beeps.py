import numpy as np
import matplotlib.pyplot as plt

# Load the data from the file
file_path = 'audio_data.txt'
with open(file_path, 'r') as file:
    audio_samples = [int(line.strip()) for line in file]

# Calculate the rate of change in amplitude between consecutive samples
# Sliding window over data. 

analyze_start = 174
block_length = 128

analyze_samples = audio_samples[800:1500]
drastic_changes = []
window_length = 20

last_drastic_change = "None"
drastic_change_threshold = 10000

for i in range(len(analyze_samples) - window_length):
    start_ind = i
    end_ind = i + window_length

    # Record if theres a drastic change in the window
    if abs(analyze_samples[start_ind] - analyze_samples[end_ind]) > drastic_change_threshold:
        # Find current direction change
        if analyze_samples[start_ind] < analyze_samples[end_ind]:
            current_drastic_change = "up"
        else:
           current_drastic_change = "down"
        
        # Compare with last direction change
        if current_drastic_change == last_drastic_change:
            # Compare current difference with last difference
            cur_dif = abs(analyze_samples[start_ind] - analyze_samples[end_ind])
            prev_dif = abs(drastic_changes[-1][0] - drastic_changes[-1][1])
            if cur_dif > prev_dif:
                drastic_changes[-1] = [start_ind, end_ind, current_drastic_change]
            else:
                continue
        else:
            drastic_changes.append([start_ind, end_ind, current_drastic_change])
            last_drastic_change = current_drastic_change

# print("Printing drastic change indices")
print(f"There were {len(drastic_changes)} drastic changes in this window\n\n")
# for change in drastic_changes:
#     print(f"Drastic change from: {change[0]} to {change[1]} was in direction: {change[2]}\n")

# Now plot `analyze_samples` with the start and end indices of drastic changes marked
plt.figure(figsize=(10, 4))
plt.plot(analyze_samples, label='Analyzed Audio Samples')

# Mark the start and end indices of drastic changes on the plot
# for change in drastic_changes:
#     plt.axvline(x=change[0], color='green', linestyle='--', label='Start of Drastic Change')
#     plt.axvline(x=change[1], color='red', linestyle='--', label='End of Drastic Change')

# Clip drastic changes length to multiple of 16
if len(drastic_changes) > 16:
    print(f"\n\nClipping length of drastic changes. Current length: {len(drastic_changes)}\n\n")
    drastic_changes = drastic_changes[:16]
    print(f"Length of drastic changes is now: {len(drastic_changes)}")

# Mark start and end drastic changes
plt.axvline(x=drastic_changes[0][0], color='green', linestyle='--', label='Start of Drastic Change')
plt.axvline(x=drastic_changes[0][1], color='red', linestyle='--', label='End of Drastic Change')
plt.axvline(x=drastic_changes[-1][0], color='green', linestyle='--', label='Start of Drastic Change')
plt.axvline(x=drastic_changes[-1][1], color='red', linestyle='--', label='End of Drastic Change')

# Find zero crossing points for start and end
print("\nFinding zero start point\n")
start_zero_point = 0
for i in range(drastic_changes[0][0], drastic_changes[0][1] - 1):
    # Find zero crossing point
    # print(f"Left point: {analyze_samples[i]}, Right point: {analyze_samples[i + 1]}")
    if analyze_samples[i] < 0 and analyze_samples[i + 1] > 0:
        start_zero_point = i
end_zero_point = 0
print("\nFinding zero end point\n")
for i in range(drastic_changes[-1][0], drastic_changes[-1][1] - 1):
    print(f"Left point: {analyze_samples[i]}, Right point: {analyze_samples[i + 1]}")
    # Find zero crossing point
    if analyze_samples[i] > 0 and analyze_samples[i + 1] < 0:
        end_zero_point = i

print(f"Start zero point is: {start_zero_point}, End zero point is: {end_zero_point}")
fence_num = len(drastic_changes) // 16
silence_num = fence_num

expected_distance = (block_length * 2 * fence_num) + block_length * silence_num
actual_distance = end_zero_point - start_zero_point
print(f"Expected distance is: {expected_distance}, Actual distance is: {actual_distance}")


# Add labels and title to the plot
plt.xlabel('Sample Index within Analyzed Window')
plt.ylabel('Amplitude')
plt.title('Analyzed Audio Samples with Drastic Changes Marked')
plt.legend()

# Show the plot
plt.savefig("beep_detection_plot.png")