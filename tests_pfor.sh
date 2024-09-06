#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_pfor.log
#SBATCH -e ./error_pfor.log
#SBATCH -t 02:00:00

python tests_pfor.py
