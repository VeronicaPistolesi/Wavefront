import os
import subprocess
import csv
import itertools

folder_path = './build'  

param_grid_exec_2 = {
    'N': [10, 100, 1000, 2000, 3000, 5000, 10000],   
    'n_workers': [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32]
}

param_combinations_exec_2 = list(itertools.product(
    param_grid_exec_2['N'],
    param_grid_exec_2['n_workers']
))


def run_command(command):
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        output_lines = result.stdout.strip().split('\n')
        times = []
        
        for line in output_lines:
            if line.strip():
                parts = line.split()
                try:
                    time = int(parts[-2])
                    times.append(time)
                except ValueError:
                    continue
        
        if times:
            return max(times)
        else:
            return None

    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        return None

results = []

csv_headers = ['file_name', 'N', 'n_workers'] + [f'time{i}' for i in range(10)] + ['mean_time']

for file_name in os.listdir(folder_path):
    executable = os.path.join(folder_path, file_name.replace('.cpp', ''))
    
    if 'mpi' in file_name:
        continue
    else:
        if 'seq' in file_name:
            continue
        elif 'grain' in file_name:
            continue
        else:
            param_combinations = param_combinations_exec_2
            for params in param_combinations:
                times = []
                for _ in range(1):
                    command = [executable] + list(map(str, params))
                    time = run_command(command)
                    if time is not None:
                        times.append(time)
                
                mean_time = sum(times)/len(times) if times else None
                result_row = [
                    file_name,
                    params[0],  # N
                    params[1],  # n_workers
                ] + times + [mean_time]
                print(result_row, flush=True)
                results.append(result_row)

csv_file = 'tests_pfor.csv'
with open(csv_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(csv_headers)
    writer.writerows(results)

print(f"Results have been written to {csv_file}")
