import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

# Read the CSV file
file_path = 'output100MHz.csv'
data = pd.read_csv(file_path)

# Display the data to ensure it was read correctly
print(data)

# Convert time from milliseconds to seconds
data['Write Time (s)'] = data['Write Time (ms)'] / 160
data['Read Time (s)'] = data['Read Time (ms)'] / 160

# Plot Write Time
plt.figure(figsize=(12, 6))
plt.subplot(2, 2, 1)
plt.plot(data['Size'], data['Write Time (s)'], marker='o', label='Write Time')
plt.axvline(x=2048, color='blue', linestyle='--', label='2048 bytes')  # Add vertical line at 2048 bytes
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Time (seconds)')
plt.title('Write Time vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')




# Plot Write Data Rate
plt.subplot(2, 2, 3)
plt.plot(data['Size'], data['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
plt.axvline(x=2048, color='red', linestyle='--', label='2048 bytes')  # Add vertical line at 2048 bytes
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Data Rate (bytes/s)')
plt.title('Write Data Rate vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')

# Plot Read Time
plt.subplot(2, 2, 2)
plt.plot(data['Size'], data['Read Time (s)'], marker='o', label='Read Time')
plt.axvline(x=2048, color='blue', linestyle='--', label='2048 bytes')  # Add vertical line at 2048 bytes
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Time (seconds)')
plt.title('Read Time vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')


# Plot Read Data Rate
plt.subplot(2, 2, 4)
plt.plot(data['Size'], data['Read Data Rate (bytes/s)'], marker='o', color='r', label='Read Data Rate')
plt.axvline(x=2048, color='red', linestyle='--', label='2048 bytes')  # Add vertical line at 2048 bytes
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Data Rate (bytes/s)')
plt.title('Read Data Rate vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')



plt.tight_layout()
plt.show()



plt.figure(figsize=(12, 6))

# Plot Write and Read Time in one plot
plt.subplot(2, 1, 1)
plt.plot(data['Size'], data['Write Time (s)'], marker='o', label='Write Time')
plt.plot(data['Size'], data['Read Time (s)'], marker='o', label='Read Time', linestyle='--')
plt.axvline(x=2048, color='blue', linestyle='--', label='2048 bytes')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Time (seconds)')
plt.title('Write and Read Time vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')

# Plot Write and Read Data Rate in another plot
plt.subplot(2, 1, 2)
plt.plot(data['Size'], data['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
plt.plot(data['Size'], data['Read Data Rate (bytes/s)'], marker='o', color='g', linestyle='--', label='Read Data Rate')
plt.axvline(x=2048, color='red', linestyle='--', label='2048 bytes')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Data Rate (bytes/s)')
plt.title('Write and Read Data Rate vs File Size')
plt.legend()
plt.grid(True)
plt.text(0.95, 0.05, 'SPI frequency: 16 MHz', fontsize=10, color='black', transform=plt.gca().transAxes, horizontalalignment='right')

plt.tight_layout()
plt.show()