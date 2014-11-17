#!/bin/bash

# run only on local node
mpirun -host $HOSTNAME -np 4 ./bin/mpiping 100

