#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks=1
#SBATCH -o ./output_pforgrain_d_t.log
#SBATCH -e ./error_pforgrain_d_t.log
#SBATCH -t 02:00:00
#SBATCH -j test_pforgrain_d_t

python tests_pforgrain_d_t.py
