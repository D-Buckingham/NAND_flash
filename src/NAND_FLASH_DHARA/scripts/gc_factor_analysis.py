import re
from datetime import datetime
import matplotlib.pyplot as plt
import math

# Function to read the log file and extract relevant timestamps
def extract_timestamps(file_path):
    with open(file_path, 'r') as file:
        log_data = file.readlines()
    timestamps = [re.findall(r'(\d{2}:\d{2}:\d{2}:\d{3})', line)[0] for line in log_data if 'Current block' in line]
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
    data_rates = [(block_size / (td / 1000)) for td in time_differences if td > 0]  # B/s
    return data_rates

# Function to extract the gc_factor from the file name
def extract_gc_factor(file_label):
    match = re.search(r'(\d+)', file_label)
    return match.group(1) if match else 'unknown'

# Function to plot the frequency of time differences
def plot_time_differences_histogram(time_differences, file_label, ax):
    gc_factor = extract_gc_factor(file_label)
    ax.hist(time_differences, bins=50, range=(0, 2000))
    ax.set_title(f'Frequency of Time Differences\n(gc = {gc_factor})')
    ax.set_xlabel('Time Difference (ms)')
    ax.set_ylabel('Frequency')
    ax.grid(True)
    ax.set_xlim(0, 2000)

# Function to plot time differences over timestamps
def plot_time_differences_over_time(time_differences, timestamps, file_label, ax):
    gc_factor = extract_gc_factor(file_label)
    ax.plot(range(len(time_differences)), time_differences, marker='o', linestyle='-')
    ax.set_title(f'Time Differences Over Timestamps\n(gc = {gc_factor})')
    ax.set_xlabel('Index')
    ax.set_ylabel('Time Difference (ms)')
    ax.grid(True)

# Function to plot data rate
def plot_data_rate(data_rates, file_label, ax):
    gc_factor = extract_gc_factor(file_label)
    ax.plot(range(len(data_rates)), data_rates, marker='o', linestyle='-')
    ax.set_title(f'Data Rate Over Time\n(gc = {gc_factor})')
    ax.set_xlabel('Index')
    ax.set_ylabel('Data Rate (B/s)')
    ax.set_ylim(50000,900000)
    ax.grid(True)

# Main function to execute the script for multiple files
def main():
    file_paths = ['log_data0.txt', 'log_data1.txt', 'log_data5.txt', 'log_data10.txt', 'log_data15.txt', 'log_data20.txt', 'log_data30.txt', 'log_data40.txt']  # Replace with your log file paths
    block_size = 64*2056  # bytes
    
    num_files = len(file_paths)
    num_cols = 3
    num_rows = math.ceil(num_files / num_cols)
    
    fig_hist, axs_hist = plt.subplots(num_rows, num_cols, figsize=(num_cols * 5, num_rows * 5), sharey=True)
    fig_time, axs_time = plt.subplots(num_rows, num_cols, figsize=(num_cols * 5, num_rows * 5), sharey=True)
    fig_rate, axs_rate = plt.subplots(num_rows, num_cols, figsize=(num_cols * 5, num_rows * 5), sharey=True)

    axs_hist = axs_hist.flatten()
    axs_time = axs_time.flatten()
    axs_rate = axs_rate.flatten()

    for i, file_path in enumerate(file_paths):
        timestamps = extract_timestamps(file_path)
        if not timestamps:
            print(f"No relevant timestamps found in {file_path}.")
            continue

        datetime_stamps = convert_to_datetime(timestamps)
        time_differences = calculate_time_differences(datetime_stamps)
        data_rates = calculate_data_rate(time_differences, block_size)
        
        if time_differences:
            plot_time_differences_histogram(time_differences, file_path, axs_hist[i])
            plot_time_differences_over_time(time_differences, datetime_stamps, file_path, axs_time[i])
        else:
            print(f"Not enough timestamps to calculate differences in {file_path}.")

        if data_rates:
            plot_data_rate(data_rates, file_path, axs_rate[i])
        else:
            print(f"Not enough data to calculate data rate in {file_path}.")
    
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
