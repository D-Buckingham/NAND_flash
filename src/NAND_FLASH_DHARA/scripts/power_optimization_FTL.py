import numpy as np
import matplotlib.pyplot as plt

# Given data
fill_times_without = np.array([52*60+9, 31*60+15, 20*60+21, 15*60+5])  # in seconds
fill_times_opt = np.array([49*60+4, 29*60+35, 19*60+22, 14*60+35])      # in seconds
power_consumption = np.array([20*1.8, 20.85*1.8, 22.3*1.8, 23.35*1.8])  # in mW
spi_speeds = np.array([2, 4, 8, 16])  # MHz

# Calculate energy consumption in mJ (milliJoules)
energy_without_opt = fill_times_without * power_consumption
energy_with_opt = fill_times_opt * power_consumption

# Calculate percent decrease in energy consumption
percent_decrease = (energy_without_opt - energy_with_opt) / energy_without_opt * 100

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(spi_speeds, percent_decrease, marker='o', linestyle='-', color='b')
plt.title('Percent Decrease in Energy Consumption vs SPI Speed')
plt.xlabel('SPI Speed (MHz)')
plt.ylabel('Percent Decrease in Energy Consumption (%)')
plt.grid(True)
plt.xticks(spi_speeds)

# Annotating the points with percentage decrease
for i, txt in enumerate(percent_decrease):
    plt.annotate(f'{txt:.2f}%', (spi_speeds[i], percent_decrease[i]), textcoords="offset points", xytext=(0,10), ha='center')

plt.savefig('percent_decrease_energy_consumption.png')
plt.show()
