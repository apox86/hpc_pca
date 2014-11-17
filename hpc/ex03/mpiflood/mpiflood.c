#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <mpi.h>

void usage(void)
{
	printf("Syntax Error\n");
	printf("usage: TODO\n");
}

int main(int argc, char *argv[])
{
	/* mpi setup  */
	int size, rank;
	char hostname[50];
	MPI_Status status;
#if SYNC == 0
	MPI_Request request;
#endif
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank); // Who am I?
	MPI_Comm_size (MPI_COMM_WORLD, &size); // How many processes?
	gethostname(hostname, 50);

	const int N = 19; // 2^N Bytes
	const unsigned long SIZE = (0x1<<N)*1024L; // [kBytes]
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t)*SIZE);
	
	for (uint32_t i=0; i<SIZE; i++)
		*data = rand() & 0xFF;

	printf("I'm process %2d out of %2d (%s)\n", rank, size, hostname);
	MPI_Barrier(MPI_COMM_WORLD);
	
	int dst, src;
	double time[2];
	double timeTotal, dataRate;
	
	uint32_t i, j;
	unsigned long msgsize;
	const int AVG = 5;
	
	for (i=0; i<=N; i++)
	{
		msgsize = (0x1 << i)*1024L; // 2,4,8,16,... kByte
		if (msgsize > SIZE)
			printf("%2d: msg too big!\n", rank);
	
		if (rank == 0)
		{
			timeTotal = 0;
			for (j=0; j<=AVG; j++)
			{
				time[0] = MPI_Wtime();
				{
					dst = 1;
#if SYNC == 1
					MPI_Send(data, msgsize, MPI_BYTE, dst, 0, MPI_COMM_WORLD);
#else
					MPI_Isend(data, msgsize, MPI_BYTE, dst, 0, MPI_COMM_WORLD, &request);
					MPI_Wait(&request, &status);
#endif
					src = 1;
					MPI_Recv(data, 1, MPI_BYTE, src, 0, MPI_COMM_WORLD, &status);
				}
				time[1] = MPI_Wtime();
				
				timeTotal += time[1]-time[0];
				//printf("%i %ld %lf %lf\n", i, msgsize, timeTotal, dataRate);
			}
			timeTotal /= AVG;
			dataRate = ((double)msgsize/(double)timeTotal)/1e6; //[MB/s]

			// gnuplot
			printf("%ld %lf %lf\n", msgsize/1024, timeTotal, dataRate);
			fflush(stdout);
		}
		else if (rank == 1)
		{
			for (j=0; j<=AVG; j++)
			{
				src = MPI_ANY_SOURCE;
				MPI_Recv(data, msgsize, MPI_BYTE, src, 0, MPI_COMM_WORLD, &status);

				dst = status.MPI_SOURCE;
				MPI_Send(data, 1, MPI_BYTE, dst, 0, MPI_COMM_WORLD);
			}
		}
		else
		{
			printf("%2d: error: i'm useless..\n", rank);
		}
	}
	
	free(data);
	//printf("%2d: done\n", rank);
	MPI_Finalize();

	return 0;
}

