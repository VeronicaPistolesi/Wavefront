#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_pforgrain_d_t.log
#SBATCH -e ./error_pforgrain_d_t.log
#SBATCH -t 02:00:00

python tests_pforgrain_d_t.py
