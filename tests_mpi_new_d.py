import os
import subprocess
import csv
import itertools

folder_path = './build'  


param_grid_mpi = {
    'n_workers': [2, 3, 4, 5, 6, 7, 8],
    'N': [10000],
    'threads': [1, 2]
}


param_combinations_mpi = list(itertools.product(
    param_grid_mpi['n_workers'],
    param_grid_mpi['N'],
    param_grid_mpi['threads']
))


def run_command(command, threads_per_node):
    try:
        env = os.environ.copy()
        env['OMP_NUM_THREADS'] = str(threads_per_node)
        result = subprocess.run(command, capture_output=True, text=True, check=True, env=env)
        output_lines = result.stdout.strip().split('\n')[::2]
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

csv_headers = ['file_name', 'N', 'n_workers', 'threads'] + [f'time{i}' for i in range(10)] + ['mean_time']

for file_name in os.listdir(folder_path):
    executable = os.path.join(folder_path, file_name.replace('.cpp', ''))
    
    if 'new' in file_name:
        for params in param_combinations_mpi:
            times = []
            for _ in range(1):  
                command = ['mpiexec', '-n', str(params[0]), executable, str(params[1])]
                time = run_command(command, str(params[2]))
                if time is not None:
                    times.append(time)
            
            mean_time = sum(times)/len(times) if times else None
            result_row = [
                file_name,
                params[1],  # N
                params[0],  # n_workers
                params[2]   # threads
            ] + times + [mean_time]
            print(result_row, flush=True)
            results.append(result_row)

csv_file = 'tests_mpi_new_d.csv'
with open(csv_file, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(csv_headers)
    writer.writerows(results)

print(f"Results have been written to {csv_file}")
