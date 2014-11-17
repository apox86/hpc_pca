#!/bin/bash

# run only on local node
mpirun -host $HOSTNAME -np 2 ./bin/mpiflood_SYNC
mpirun -host $HOSTNAME -np 2 ./bin/mpiflood_ASYNC

