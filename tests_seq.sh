#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_seq.log
#SBATCH -e ./error_seq.log
#SBATCH -t 02:00:00

python tests_seq.py
