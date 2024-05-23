import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

# Read the CSV file
file_path1 = 'output100MHz.csv'
file_path2 = 'output100MHz_latency_improved.csv'
data1 = pd.read_csv(file_path1)
data2 = pd.read_csv(file_path2)

# Display the data1 to ensure it was read correctly
print(data1)
print(data2)


# Convert time from milliseconds to seconds
data1['Write Time (s)'] = data1['Write Time (ms)'] / 1000
data1['Read Time (s)'] = data1['Read Time (ms)'] / 1000
data2['Write Time (s)'] = data2['Write Time (ms)'] / 1000
data2['Read Time (s)'] = data2['Read Time (ms)'] / 1000


# Calculate percentage improvement in latency
data1['Write Time Improvement (%)'] = ((data1['Write Time (s)'] - data2['Write Time (s)']) / data1['Write Time (s)']) * 100
data1['Read Time Improvement (%)'] = ((data1['Read Time (s)'] - data2['Read Time (s)']) / data1['Read Time (s)']) * 100


# Plot Write Time
plt.figure(figsize=(12, 6))
plt.subplot(2, 2, 1)
plt.plot(data1['Size'], data1['Write Time (s)'], marker='o', label='Write Time')
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
plt.plot(data1['Size'], data1['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
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
plt.plot(data1['Size'], data1['Read Time (s)'], marker='o', label='Read Time')
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
plt.plot(data1['Size'], data1['Read Data Rate (bytes/s)'], marker='o', color='r', label='Read Data Rate')
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
plt.plot(data1['Size'], data1['Write Time (s)'], marker='o', label='Write Time')
plt.plot(data1['Size'], data1['Read Time (s)'], marker='o', label='Read Time', linestyle='--')
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
plt.plot(data1['Size'], data1['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
plt.plot(data1['Size'], data1['Read Data Rate (bytes/s)'], marker='o', color='g', linestyle='--', label='Read Data Rate')
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



###########################plot improved #######################
# Plot Write Time
plt.figure(figsize=(12, 6))
plt.subplot(2, 2, 1)
plt.plot(data2['Size'], data2['Write Time (s)'], marker='o', label='Write Time')
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
plt.plot(data2['Size'], data2['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
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
plt.plot(data2['Size'], data2['Read Time (s)'], marker='o', label='Read Time')
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
plt.plot(data2['Size'], data2['Read Data Rate (bytes/s)'], marker='o', color='r', label='Read Data Rate')
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
plt.plot(data2['Size'], data2['Write Time (s)'], marker='o', label='Write Time')
plt.plot(data2['Size'], data2['Read Time (s)'], marker='o', label='Read Time', linestyle='--')
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
plt.plot(data2['Size'], data2['Write Data Rate (bytes/s)'], marker='o', color='r', label='Write Data Rate')
plt.plot(data2['Size'], data2['Read Data Rate (bytes/s)'], marker='o', color='g', linestyle='--', label='Read Data Rate')
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


###################### improved ##################################

# Plot Write Time Improvement
plt.subplot(2, 1, 1)
plt.plot(data1['Size'], data1['Write Time Improvement (%)'], marker='o', label='Write Time Improvement')
plt.axvline(x=2048, color='blue', linestyle='--', label='2048 bytes')
plt.xscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Improvement (%)')
plt.title('Write Time Improvement vs File Size')
plt.legend()
plt.grid(True)

# Plot Read Time Improvement
plt.subplot(2, 1, 2)
plt.plot(data1['Size'], data1['Read Time Improvement (%)'], marker='o', color='r', label='Read Time Improvement')
plt.axvline(x=2048, color='red', linestyle='--', label='2048 bytes')
plt.xscale('log')
plt.xlabel('File Size (bytes)')
plt.ylabel('Improvement (%)')
plt.title('Read Time Improvement vs File Size')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()