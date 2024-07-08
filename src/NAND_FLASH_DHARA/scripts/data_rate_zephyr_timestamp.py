import re
from datetime import datetime
import matplotlib.pyplot as plt
import math

# Function to read the log file and extract relevant timestamps
def extract_timestamps(file_path):
    with open(file_path, 'r') as file:
        log_data = file.readlines()
    timestamps = [re.findall(r'I: (\d{2}:\d{2}:\d{2}:\d{3}) - Current block: : \d+', line)[0] for line in log_data if re.findall(r'I: (\d{2}:\d{2}:\d{2}:\d{3}) - Current block: : \d+', line)]
    return timestamps

# Function to convert timestamps to datetime objects
def convert_to_datetime(timestamps):
    return [datetime.strptime(ts, '%H:%M:%S:%f') for ts in timestamps]

# Function to calculate time differences between consecutive timestamps
def calculate_time_differences(timestamps):
    time_differences = [(timestamps[i] - timestamps[i - 1]).total_seconds() * 1000
                        for i in range(1, len(timestamps))]
    return time_differences

# Function to calculate data rate
def calculate_data_rate(time_differences, block_size):
    data_rates = [(block_size / (td / 1000)) for td in time_differences if td > 0 and (block_size / (td / 1000)) <= 8000000]  # B/s, removing outliers > 8000000 B/s
    return data_rates

# Function to plot the frequency of time differences
def plot_time_differences_histogram(time_differences, file_label, ax):
    ax.hist(time_differences, bins=50, range=(0, 2000))
    ax.set_title(f'Frequency of Time Differences\n{file_label}')
    ax.set_xlabel('Time Difference (ms)')
    ax.set_ylabel('Frequency')
    ax.grid(True)
    ax.set_xlim(0, 2000)

# Function to plot time differences over timestamps
def plot_time_differences_over_time(time_differences, timestamps, file_label, ax):
    ax.plot(range(len(time_differences)), time_differences, marker='o', linestyle='-')
    ax.set_title(f'Time Differences Over Timestamps\n{file_label}')
    ax.set_xlabel('Index')
    ax.set_ylabel('Time Difference (ms)')
    ax.grid(True)

# Function to plot data rate
def plot_data_rate(data_rates, file_label, ax):
    ax.plot(range(len(data_rates)), data_rates, marker='o', linestyle='-')
    ax.set_title(f'Data Rate Over Time (SPI 16 MHz)')
    ax.set_xlabel('Index')
    ax.set_ylabel('Data Rate (B/s)')
    ax.set_ylim(50000, 1500000)
    ax.grid(True)

# Main function to execute the script for a single file
def main():
    file_path = 'log_data_16MHz_SPI.txt'  # Replace with your specific log file path
    block_size = 64 * 2056  # bytes

    fig_hist, ax_hist = plt.subplots(figsize=(5, 5))
    fig_time, ax_time = plt.subplots(figsize=(5, 5))
    fig_rate, ax_rate = plt.subplots(figsize=(5, 5))

    timestamps = extract_timestamps(file_path)
    if not timestamps:
        print(f"No relevant timestamps found in {file_path}.")
        return

    datetime_stamps = convert_to_datetime(timestamps)
    time_differences = calculate_time_differences(datetime_stamps)
    data_rates = calculate_data_rate(time_differences, block_size)
    
    if time_differences:
        plot_time_differences_histogram(time_differences, file_path, ax_hist)
        plot_time_differences_over_time(time_differences, datetime_stamps, file_path, ax_time)
    else:
        print(f"Not enough timestamps to calculate differences in {file_path}.")

    if data_rates:
        plot_data_rate(data_rates, file_path, ax_rate)
        avg_data_rate = sum(data_rates) / len(data_rates)
        print(f"Average Data Rate for {file_path}: {avg_data_rate:.2f} B/s")
    else:
        print(f"Not enough data to calculate data rate in {file_path}.")

    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
