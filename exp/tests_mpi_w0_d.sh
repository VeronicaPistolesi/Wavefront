#!/bin/bash
#SBATCH -N 8
#SBATCH -n 8
#SBATCH --cpus-per-task=32
#SBATCH -o ./output_mpi_w0_d.log
#SBATCH -e ./error_mpi_w0_d.log
#SBATCH -t 02:00:00

python exp/tests_mpi_w0_d.py
