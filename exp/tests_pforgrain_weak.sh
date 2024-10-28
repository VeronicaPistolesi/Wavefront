#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_pforgrain_weak.log
#SBATCH -e ./error_pforgrain_weak.log
#SBATCH -t 02:00:00

python exp/tests_pforgrain_weak.py
