#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_pforgrain.log
#SBATCH -e ./error_pforgrain.log
#SBATCH -t 02:00:00

python tests_pforgrain.py
