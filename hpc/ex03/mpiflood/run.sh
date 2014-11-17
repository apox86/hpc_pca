#!/bin/bash

rm ./data/*
echo $HOSTNAME

# run only on local node
mpirun -host $HOSTNAME -np 2 ./bin/mpiflood_SYNC >> ./data/sync_local.log
mpirun -host $HOSTNAME -np 2 ./bin/mpiflood_ASYNC >> ./data/async_local.log

# run on different nodes
mpirun -host creek03,creek08 -np 2 ./bin/mpiflood_SYNC >> ./data/sync.log
mpirun -host creek03,creek08 -np 2 ./bin/mpiflood_ASYNC >> ./data/async.log

gnuplot plot.gnuplot

