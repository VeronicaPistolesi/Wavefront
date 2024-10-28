#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_seq_weak.log
#SBATCH -e ./error_seq_weak.log
#SBATCH -t 02:00:00

python exp/tests_seq_weak.py
