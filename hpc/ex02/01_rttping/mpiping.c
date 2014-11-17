#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <mpi.h>

void usage(void)
{
	printf("Syntax Error\n");
	printf("usage: ./mpiping <no. of messages>\n");
}

int main(int argc, char *argv[])
{
	unsigned int i;
	int size, rank;
	char hostname[50];
	MPI_Status status;

	/* mpi setup  */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank); // Who am I?
	MPI_Comm_size (MPI_COMM_WORLD, &size); // How many processes?
	gethostname(hostname, 50);

	if (argc != 2)
	{
		usage();
		MPI_Finalize();
		return 0;
	}

	int m = atoi(argv[1]);
	if (m <= 0)
	{
		m = 10;
		printf("Warning: Invalid parameter, using default value m=10\n");
	}

	int dst, src;
	double time[2];
	double timeTotal, timePerMsg;
	int odata, idata;
	odata = 1234;

	//printf("Hello Parallel World!\n");
	//printf("I'm process %2d out of %2d (%s)\n", rank, size, hostname);
	//MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) // Master
	{
		time[0] = MPI_Wtime();
		for (i=0; i<m; i++)
		{
			dst = 1;
			MPI_Send(&odata, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);
			//printf("%2d: %i sent\n", rank, odata);

			src = size-1;
			MPI_Recv(&idata, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
			//printf("%2d: %i received\n", rank, idata);
		}
		time[1] = MPI_Wtime();
#if 0
		// debug
		printf("\x1B[31m");
		printf("runtime=%lf\n", time[1]-time[0]);
		printf("\x1B[0m");
#else
		// gnuplot
		timeTotal = time[1]-time[0];
		timePerMsg = timeTotal / m;
		printf("#Nodes, Messages, time total, time per msg\n");
		printf("%i %i %lf %lf\n", size, m, timeTotal, timePerMsg);
#endif
	}
	else // Slave
	{
		for (i=0; i<m; i++)
		{
			src = MPI_ANY_SOURCE;
			MPI_Recv(&idata, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
			//printf("%2d: %i received\n", rank, idata);

			dst = (rank+1) % size;
			//printf("%2d: nextdst=%i\n", rank, dst);
			MPI_Send(&odata, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);
			//printf("%2d: %i sent\n", rank, odata);
		}
	}
	
	//printf("%2d: done\n", rank);
	MPI_Finalize();

	return 0;
}

