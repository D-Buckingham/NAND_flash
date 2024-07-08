import re
from datetime import datetime
import matplotlib.pyplot as plt

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

# Function to plot the frequency of time differences
def plot_time_differences_histogram(time_differences, file_label):
    plt.figure(figsize=(10, 5))
    plt.hist(time_differences, bins=50, range=(0, 1000), color='blue', alpha=0.7)
    plt.title(f'Frequency of Time Differences ({file_label})')
    plt.xlabel('Time Difference (ms)')
    plt.ylabel('Frequency')
    plt.grid(True)
    plt.xlim(0, 1000)
    plt.tight_layout()
    plt.show()

# Function to plot time differences over timestamps
def plot_time_differences_over_time(time_differences, timestamps, file_label):
    plt.figure(figsize=(10, 5))
    plt.plot(timestamps[1:], time_differences, marker='o', linestyle='-')
    plt.title(f'Time Differences Over Timestamps ({file_label})')
    plt.xlabel('Timestamp')
    plt.ylabel('Time Difference (ms)')
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.xlim(timestamps[1], timestamps[min(1000, len(timestamps)-1)])
    plt.tight_layout()
    plt.show()

# Main function to execute the script for multiple files
def main():
    file_paths = ['log_data0.txt', 'log_data1.txt', 'log_data5.txt', 'log_data10.txt','log_data15.txt', 'log_data20.txt', 'log_data30.txt', 'log_data40.txt']  # Replace with your log file paths

    for file_path in file_paths:
        timestamps = extract_timestamps(file_path)
        if not timestamps:
            print(f"No relevant timestamps found in {file_path}.")
            continue

        datetime_stamps = convert_to_datetime(timestamps)
        time_differences = calculate_time_differences(datetime_stamps)
        
        if time_differences:
            plot_time_differences_histogram(time_differences, file_path)
            plot_time_differences_over_time(time_differences, datetime_stamps, file_path)
        else:
            print(f"Not enough timestamps to calculate differences in {file_path}.")

if __name__ == '__main__':
    main()
