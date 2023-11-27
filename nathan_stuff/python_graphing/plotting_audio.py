# Here's how you can modify the code to draw vertical lines at each "DRASTIC_INCREASE":
import matplotlib.pyplot as plt

file_path = 'audio_data.txt'

fence_start_indices = []
fence_end_indices = []

with open(file_path, 'r') as file:
    raw_samples = []
    increase_indices = []
    for i, line in enumerate(file):
        line = line.strip()
        if line == "FENCE_START":
            fence_start_indices.append(len(raw_samples))
        elif line == "FENCE_END":
            fence_end_indices.append(len(raw_samples))
        else:
            value = int(line)
            raw_samples.append(value)
        
audio_samples = raw_samples

# Zip fence lists
fence_boundaries = list(zip(fence_start_indices, fence_end_indices))

print("Printing fences boundaries")
for i in range(len(fence_boundaries)):
    print(f"Fence: {i}: Start: {fence_boundaries[i][0]}, End: {fence_boundaries[i][1]}. Fence length: {fence_boundaries[i][1] - fence_boundaries[i][0]}")

# Plot the audio samples
# analyze_start = 9000
# analyze_end = 13000

analyze_start = 0
analyze_end = len(audio_samples)

plt.figure(figsize=(10, 4))
plt.plot(audio_samples[analyze_start:analyze_end])


# Draw vertical lines for each fence start and fence end
for idx in fence_start_indices:
    if idx > analyze_start and idx < analyze_end:
        plt.axvline(x=idx - analyze_start, color='green', linestyle='--', linewidth=1)
for idx in fence_end_indices:
    if idx > analyze_start and idx < analyze_end:
        plt.axvline(x=idx - analyze_start, color='red', linestyle='--', linewidth=1)

plt.title('Audio Samples with Drastic Increases Marked')
plt.xlabel('Sample Index')
plt.ylabel('Sample Value')
plt.savefig("audioplot.png")
plt.show()

