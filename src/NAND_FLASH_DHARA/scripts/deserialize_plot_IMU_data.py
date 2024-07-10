import struct
import os
import matplotlib.pyplot as plt

# Define the struct formats
imu_format = '<I6h'  # uint32_t timestamp, 6 int16_t for gyro and acc
mag_format = '<I3h'  # uint32_t timestamp, 3 int16_t for raw mag data
quat_format = '<I4e' # uint32_t timestamp, 4 float for raw quaternion data

# Define the sample types
SAMPLE_IMU = 0
SAMPLE_MAG = 2
SAMPLE_QUAT = 1

def deserialize_imu(data):
    imu_data = struct.unpack(imu_format, data)
    imu_dict = {
        'timestamp': imu_data[0],
        'gyro_raw': imu_data[1:4],
        'acc_raw': imu_data[4:7]
    }
    return imu_dict

def deserialize_mag(data):
    mag_data = struct.unpack(mag_format, data)
    mag_dict = {
        'timestamp': mag_data[0],
        'raw': mag_data[1:4]
    }
    return mag_dict

def deserialize_quat(data):
    quat_data = struct.unpack(quat_format, data)
    quat_dict = {
        'timestamp': quat_data[0],
        'raw': quat_data[1:5]
    }
    return quat_dict

def deserialize_sample(data):
    sample_type = data[0]
    if sample_type == SAMPLE_IMU:
        return deserialize_imu(data[1:])
    elif sample_type == SAMPLE_MAG:
        return deserialize_mag(data[1:])
    elif sample_type == SAMPLE_QUAT:
        return deserialize_quat(data[1:])
    else:
        return None

def concatenate_binary_files(file_paths, output_path):
    with open(output_path, 'wb') as outfile:
        for file_path in file_paths:
            with open(file_path, 'rb') as infile:
                outfile.write(infile.read())

def read_binary_file(file_path):
    imu_samples = []
    mag_samples = []
    quat_samples = []
    with open(file_path, 'rb') as file:
        data = file.read()
        
    while data:
        sample_type = data[0]
        data = data[1:]
        
        if sample_type == SAMPLE_IMU:
            sample_size = struct.calcsize(imu_format)
        elif sample_type == SAMPLE_MAG:
            sample_size = struct.calcsize(mag_format)
        elif sample_type == SAMPLE_QUAT:
            sample_size = struct.calcsize(quat_format)
        else:
            break

        if len(data) < sample_size:
            break

        sample_data = data[:sample_size]
        sample = deserialize_sample(bytes([sample_type]) + sample_data)
        if sample_type == SAMPLE_IMU:
            imu_samples.append(sample)
        elif sample_type == SAMPLE_MAG:
            mag_samples.append(sample)
        elif sample_type == SAMPLE_QUAT:
            quat_samples.append(sample)
        data = data[sample_size:]
    return imu_samples, mag_samples, quat_samples

# Plotting functions
def plot_imu_data(samples):
    timestamps = [sample['timestamp'] for sample in samples]
    gyro_data = [sample['gyro_raw'] for sample in samples]
    acc_data = [sample['acc_raw'] for sample in samples]

    if not timestamps or not gyro_data or not acc_data:
        print("No IMU data available to plot.")
        return

    fig, axs = plt.subplots(2, 1, figsize=(12, 8))
    
    # Gyroscope data
    axs[0].plot(timestamps, [gyro[0] for gyro in gyro_data], label='Gyro X')
    axs[0].plot(timestamps, [gyro[1] for gyro in gyro_data], label='Gyro Y')
    axs[0].plot(timestamps, [gyro[2] for gyro in gyro_data], label='Gyro Z')
    axs[0].set_title('Gyroscope Data')
    axs[0].set_xlabel('Timestamp')
    axs[0].set_ylabel('Gyro Raw')
    axs[0].legend()
    axs[0].grid(True)
    
    # Accelerometer data
    axs[1].plot(timestamps, [acc[0] for acc in acc_data], label='Acc X')
    axs[1].plot(timestamps, [acc[1] for acc in acc_data], label='Acc Y')
    axs[1].plot(timestamps, [acc[2] for acc in acc_data], label='Acc Z')
    axs[1].set_title('Accelerometer Data')
    axs[1].set_xlabel('Timestamp')
    axs[1].set_ylabel('Acc Raw')
    axs[1].legend()
    axs[1].grid(True)

    plt.tight_layout()
    plt.show()

def plot_mag_data(samples):
    timestamps = [sample['timestamp'] for sample in samples]
    mag_data = [sample['raw'] for sample in samples]

    if not timestamps or not mag_data:
        print("No Magnetometer data available to plot.")
        return

    plt.figure(figsize=(12, 6))
    plt.plot(timestamps, [mag[0] for mag in mag_data], label='Mag X')
    plt.plot(timestamps, [mag[1] for mag in mag_data], label='Mag Y')
    plt.plot(timestamps, [mag[2] for mag in mag_data], label='Mag Z')
    plt.title('Magnetometer Data')
    plt.xlabel('Timestamp')
    plt.ylabel('Mag Raw')
    plt.legend()
    plt.grid(True)
    plt.show()

def plot_quat_data(samples):
    timestamps = [sample['timestamp'] for sample in samples]
    quat_data = [sample['raw'] for sample in samples]

    if not timestamps or not quat_data:
        print("No Quaternion data available to plot.")
        return

    plt.figure(figsize=(12, 6))
    plt.plot(timestamps, [quat[0] for quat in quat_data], label='Quat W')
    plt.plot(timestamps, [quat[1] for quat in quat_data], label='Quat X')
    plt.plot(timestamps, [quat[2] for quat in quat_data], label='Quat Y')
    plt.plot(timestamps, [quat[3] for quat in quat_data], label='Quat Z')
    plt.title('Quaternion Data')
    plt.xlabel('Timestamp')
    plt.ylabel('Quat Raw')
    plt.legend()
    plt.grid(True)
    plt.show()

# Function to plot timestamp differences
def plot_timestamp_differences(samples, sample_type):
    timestamps = [sample['timestamp'] for sample in samples]
    if not timestamps:
        print(f"No timestamps available to plot for {sample_type}.")
        return
    
    timestamp_differences = [t2 - t1 for t1, t2 in zip(timestamps[:-1], timestamps[1:])]

    plt.figure(figsize=(12, 6))
    plt.plot(timestamps[1:], timestamp_differences, marker='o', linestyle='-')
    plt.title(f'Timestamp Differences ({sample_type})')
    plt.xlabel('Timestamp')
    plt.ylabel('Difference (ms)')
    plt.grid(True)
    plt.show()

# Function to plot differences between IMU and Quaternion timestamps
def plot_imu_quat_timestamp_differences(imu_samples, quat_samples):
    imu_timestamps = [sample['timestamp'] for sample in imu_samples]
    quat_timestamps = [sample['timestamp'] for sample in quat_samples]

    min_length = min(len(imu_timestamps), len(quat_timestamps))
    timestamp_differences = [imu_timestamps[i] - quat_timestamps[i] for i in range(min_length)]

    plt.figure(figsize=(12, 6))
    plt.plot(range(min_length), timestamp_differences, marker='o', linestyle='-')
    plt.title('IMU and Quaternion Timestamp Differences')
    plt.xlabel('Index')
    plt.ylabel('Difference (ms)')
    plt.grid(True)
    plt.show()

# Function to plot all timestamp differences together
def plot_all_timestamp_differences(imu_samples, mag_samples, quat_samples):
    imu_timestamps = [sample['timestamp'] for sample in imu_samples]
    mag_timestamps = [sample['timestamp'] for sample in mag_samples]
    quat_timestamps = [sample['timestamp'] for sample in quat_samples]

    imu_differences = [t2 - t1 for t1, t2 in zip(imu_timestamps[:-1], imu_timestamps[1:])]
    mag_differences = [t2 - t1 for t1, t2 in zip(mag_timestamps[:-1], mag_timestamps[1:])]
    quat_differences = [t2 - t1 for t1, t2 in zip(quat_timestamps[:-1], quat_timestamps[1:])]

    plt.figure(figsize=(12, 6))
    plt.plot(imu_timestamps[1:], imu_differences, marker='o', linestyle='-', label='IMU Differences')
    plt.plot(mag_timestamps[1:], mag_differences, marker='x', linestyle='-', label='Mag Differences')
    plt.plot(quat_timestamps[1:], quat_differences, marker='s', linestyle='-', label='Quat Differences')
    plt.title('All Timestamp Differences')
    plt.xlabel('Timestamp')
    plt.ylabel('Difference (ms)')
    plt.legend()
    plt.grid(True)
    plt.show()

# Example usage
input_files = ['Motion_tracker_data/REC_22/1']#, 'Motion_tracker_data/REC_4/2'
concatenated_file_path = 'Motion_tracker_data/REC_1/concatenated.bin'

concatenate_binary_files(input_files, concatenated_file_path)
imu_samples, mag_samples, quat_samples = read_binary_file(concatenated_file_path)

# Plotting the data
plot_imu_data(imu_samples)
plot_mag_data(mag_samples)
plot_quat_data(quat_samples)

# Plotting the timestamp differences
plot_timestamp_differences(imu_samples, "IMU")
plot_timestamp_differences(mag_samples, "Magnetometer")
plot_timestamp_differences(quat_samples, "Quaternion")

# Plotting the differences between IMU and Quaternion timestamps
plot_imu_quat_timestamp_differences(imu_samples, quat_samples)

# Plotting all timestamp differences together
plot_all_timestamp_differences(imu_samples, mag_samples, quat_samples)
