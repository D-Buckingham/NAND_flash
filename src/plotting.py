import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
file_path = 'output.csv'
data = pd.read_csv(file_path)

# Display the data to ensure it was read correctly
print(data)

# Convert time from milliseconds to seconds
data['Write Time (s)'] = data['Write Time (ms)'] / 1000
data['Read Time (s)'] = data['Read Time (ms)'] / 1000

# Plot Write Time
plt.figure(figsize=(12, 6))
plt.subplot(2, 2, 1)
plt.plot(data['Size'], data['Write Time (s)'], marker='o', label='Write Time')
plt.xscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Time (seconds)')
plt.title('Write Time vs File Size')
plt.legend()
plt.grid(True)

# Plot Write Data Rate
plt.subplot(2, 2, 2)
plt.plot(data['Size'], data['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Data Rate (bytes/s)')
plt.title('Write Data Rate vs File Size')
plt.legend()
plt.grid(True)

# Plot Read Time
plt.subplot(2, 2, 3)
plt.plot(data['Size'], data['Read Time (s)'], marker='o', label='Read Time')
plt.xscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Time (seconds)')
plt.title('Read Time vs File Size')
plt.legend()
plt.grid(True)

# Plot Read Data Rate
plt.subplot(2, 2, 4)
plt.plot(data['Size'], data['Read Data Rate (bytes/s)'], marker='o', color='r', label='Read Data Rate')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Data Rate (bytes/s)')
plt.title('Read Data Rate vs File Size')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()
