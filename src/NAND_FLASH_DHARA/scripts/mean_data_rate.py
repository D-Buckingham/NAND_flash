import matplotlib.pyplot as plt
import numpy as np

# Example data: mean data rates for different SPI speeds
spi_speeds = np.array([2, 4, 8, 16])  # MHz
data_rates = np.array([184422.42, 319014.16, 494071.19, 723596.40])  # B/s

# Calculate the percentage of possible speed
max_speeds = spi_speeds * 1e6  # Convert MHz to Hz
percentages = 8*(data_rates / max_speeds) * 100

# Plot 1: Mean Data Rates vs. SPI Speeds
plt.figure(figsize=(10, 5))
plt.plot(spi_speeds, data_rates, marker='o', linestyle='-')
plt.title('Mean Data Rates vs. SPI Speeds')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Data Rate (B/s)')
plt.grid(True)
plt.show()

# Plot 2: Percentage of Possible Speed vs. SPI Speeds
plt.figure(figsize=(10, 5))
plt.plot(spi_speeds, percentages, marker='o', linestyle='-')
plt.title('Percentage of Possible Speed vs. SPI Speeds')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Percentage of Possible Speed (%)')
plt.grid(True)
plt.show()
