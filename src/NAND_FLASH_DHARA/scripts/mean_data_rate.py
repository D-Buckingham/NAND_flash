import matplotlib.pyplot as plt
import numpy as np

# Example data: mean data rates for different SPI speeds
spi_speeds = np.array([2, 4, 8, 16])  # MHz
data_rates = np.array([184422.42, 319014.16, 494071.19, 723596.40])  # B/s

# Hypothetical power consumption values for different SPI speeds (in milliwatts)
power_consumption = np.array([19.35*1.8, 19.94*1.8, 20.81*1.8, 21.89*1.8])  # mW
#power_consumption = np.array([20*1.8, 20.85*1.8, 22.3*1.8, 23.35*1.8])

fill_times = np.array([ 49*60+43, 28*60+50, 19*60+22, 12*60+58])

# Calculate the total energy consumed in watt-hours
energy_wh = power_consumption * fill_times / 3600  # mWh
energy_mah = (energy_wh) / 1.8  # mAh


# Calculate the percentage of possible speed
max_speeds = spi_speeds * 1e6  # Convert MHz to Hz
percentages = 8 * (data_rates / max_speeds) * 100

data_rates /= 1000

# Plot 1: Mean Data Rates vs. SPI Speeds
plt.figure(figsize=(10, 5))
plt.plot(spi_speeds, data_rates, marker='o', linestyle='-')
plt.title('Mean Data Rates vs. SPI Speeds')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Data Rate (kB/s)')
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

# Plot 3: Power Consumption vs. SPI Speeds
plt.figure(figsize=(10, 5))
plt.plot(spi_speeds, power_consumption, marker='o', linestyle='-')
plt.title('Power Consumption vs. SPI Speeds')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Power Consumption (mW)')
plt.grid(True)
plt.show()


# Plot 4: Total Energy Consumption (mAh) vs. SPI Speeds
plt.figure(figsize=(10, 5))
plt.plot(spi_speeds, energy_mah, marker='o', linestyle='-')
plt.title('Total Energy Consumption to Fill Flash vs. SPI Speeds')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Total Energy Consumption (mAh)')
plt.grid(True)
plt.show()