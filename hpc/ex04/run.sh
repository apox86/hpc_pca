#!/bin/bash

rm ./data/*
echo $HOSTNAME

ALLCREEKS=creek01,creek02,creek03,creek04,creek05,creek06,creek07,creek08

for i in `seq 2 2 16`
do
	mpirun -host $HOSTNAME -np $i ./ex04 2048 1 >> ./data/42.log
	echo $i
done

for i in 128 256 512 1024 2048 4096 8192
do
	mpirun -host $HOSTNAME -np 16 ./ex04 $i 1 >> ./data/43.log
	echo $i
done

