#!/bin/bash
#SBATCH -N 8
#SBATCH --ntasks=8
#SBATCH -o ./output_mpi_new.log
#SBATCH -e ./error_mpi_new.log
#SBATCH -t 02:00:00

python tests_mpi_new.py
