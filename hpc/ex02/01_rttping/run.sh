#!/bin/bash

rm ./data/*

ALLCREEKS=creek01,creek02,creek03,creek04,creek05,creek06,creek07,creek08
OPT12NODES=creek01,creek02,creek03

ME=$HOSTNAME
echo "## I am $ME"

# run only local
#mpirun -host $ME -np 4 ./bin/mpiping 100
#MPIHOSTS=$ME

# run global
MPIHOSTS=$ALLCREEKS
for i in `seq 2 2 24`
do
	for j in 10 50 100 250 500
	do
		mpirun -host $MPIHOSTS -np $i ./bin/mpiping $j >> ./data/result$j.log
		echo "## $i, $j"
	done
done

MPIHOSTS=$OPT12NODES
for j in 10 50 100 250 500
do
	mpirun -host $MPIHOSTS -np 12 ./bin/mpiping $j >> ./data/opt_result$j.log
	echo "## 12(opt), $j"
done

echo "## gnuplot"
gnuplot plot.gnuplot

