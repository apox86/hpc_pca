#!/bin/bash

# run only on local node
mpirun -host $HOSTNAME -np 2 ./ex04 256 1

