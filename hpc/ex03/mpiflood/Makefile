PRJ = mpiflood
SRC = $(PRJ).c
CFLAGS = -Wall -std=gnu99

all:
	mpicc $(CFLAGS) $(SRC) -D SYNC=1 -o ./bin/$(PRJ)_SYNC
	mpicc $(CFLAGS) $(SRC) -D SYNC=0 -o ./bin/$(PRJ)_ASYNC


clean:
	rm ./bin/*

