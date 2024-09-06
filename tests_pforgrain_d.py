import os
import subprocess
import csv
import itertools

folder_path = './build'  

param_grid_exec_3 = {
    'N': [10000],   
    'n_workers': [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32],
    'chunks_per_worker': [5, 20, 100]
}

param_grid_exec_3['n_workers']=param_grid_exec_3['n_workers'][::-1]

param_combinations_exec_3 = list(itertools.product(
    param_grid_exec_3['N'],
    param_grid_exec_3['n_workers'],
    param_grid_exec_3['chunks_per_worker']
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

csv_headers = ['file_name', 'N', 'n_workers', 'chunks_per_worker'] + [f'time{i}' for i in range(10)] + ['mean_time']

for file_name in os.listdir(folder_path):
    executable = os.path.join(folder_path, file_name.replace('.cpp', ''))

    if 'grain' in file_name:
        param_combinations = param_combinations_exec_3
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
                params[2],  # chunks_per_worker
            ] + times + [mean_time]
            print(result_row, flush=True)
            results.append(result_row)

csv_file = 'tests_pforgrain_d.csv'
with open(csv_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(csv_headers)
    writer.writerows(results)

print(f"Results have been written to {csv_file}")
