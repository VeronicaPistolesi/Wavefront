#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_seq_s.log
#SBATCH -e ./error_seq_s.log
#SBATCH -t 02:00:00

python tests_seq_s.py

