#!/bin/bash
#$ -cwd
#$ -N kscc
#$ -q kscc.q

./virus < virus.in > virus.out
