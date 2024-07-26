def count_132_intervals(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
    
    erase_count_intervals = []
    current_count = 0
    between_erase_counts = False
    
    for line in lines:
        if "Current erase count" in line:
            if between_erase_counts:
                erase_count_intervals.append(current_count)
                current_count = 0
            between_erase_counts = True
        elif between_erase_counts:
            current_count += line.count("132")
    
    # Append the last interval if the file doesn't end with an "erase count"
    if between_erase_counts:
        erase_count_intervals.append(current_count)
    
    total_count = sum(erase_count_intervals)
    mean_count = total_count / len(erase_count_intervals) if erase_count_intervals else 0
    
    return total_count, mean_count

filename = 'buffer16test.txt'  # Replace with your actual file name
total_count, mean_count = count_132_intervals(filename)
print(f"Total count of 132: {total_count}")
print(f"Mean count of 132: {mean_count}")
