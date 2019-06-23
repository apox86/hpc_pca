#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0

#define DEBUG TRUE

#define SIZE 2048

#define INIT_STYLE_SIMPLE 0
#define INIT_STYLE_RANDOM 1

long calcRuntime(struct timeval *begin, struct timeval *end);

double drand();

void initMatrixRandom(double* matr, int dim, int random);
void initMatrix(double* matr, int dim, int multiply);

void doBoss(int size, int matrSize, int initStyle);
void doWorker(int rank, int size, int matrSize);

double* reorderMatrix(double *matr, unsigned size);

void plot(double* mat, int dim);

/**MAIN**/
int main(int argc, char** argv){

	int rank, size;

	int matrSize = SIZE;
	int initStyle = INIT_STYLE_RANDOM;

	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc == 2){
		matrSize = atoi(argv[1]);
	}else if(argc == 3){
		matrSize = atoi(argv[1]);
		initStyle = atoi(argv[2]);
		if(initStyle == 0){
			initStyle = INIT_STYLE_SIMPLE;
		}else{
			initStyle = INIT_STYLE_RANDOM;
		}
	}

	printf("%d matrsize: %d\n", rank, matrSize);

	if(rank == 0){
		//boss node;
		doBoss(size, matrSize, initStyle);
	}else{
		//worker node;
		doWorker(rank, size, matrSize);
	}


	MPI_Finalize();
	return 0;
}

/*Function: calcRuntime()*/
long calcRuntime(struct timeval *begin, struct timeval *end){
	struct timeval diff;
	diff.tv_sec = end->tv_sec - begin->tv_sec;

	if(begin->tv_usec > end->tv_usec){
		diff.tv_usec = (1e6- end->tv_usec) + begin->tv_usec;
	}else{
		diff.tv_usec = end->tv_usec - begin->tv_usec;
	}

	return (diff.tv_sec * 1e6 + diff.tv_usec);
}

/*Function: drand()*/
double drand(){
	return rand() / (RAND_MAX + 1.);
}

/*Function: initMatrixRandom()*/
void initMatrixRandom(double* matr, int dim, int random){
	int i;
	int j;
	
	for(i = 0; i <dim; i++){
		for(j = 0; j < dim; j++){
			if(random == TRUE){
				*(matr+j+i*dim) = drand();	
			}else{
				*(matr+j+i*dim) = 0.0;
			}
		}
	}
}

/*Function: initMatrix()*/
void initMatrix(double* matr, int dim, int multiply){
	int i;
	int j;
	
	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			if(multiply == TRUE){
				*(matr+j+i*dim) = i*j;	
			}else{
				*(matr+j+i*dim) = i+j;
			}
		}
	}

}

void doBoss(int size,  int matrSize,  int initStyle){
	double* matr1 = NULL;
	double* matr2 = NULL;
	double* bufferMatr = NULL;
	double* recvBuffer = NULL;
	double* matrErg = NULL;
	
	int i = 1;
	int j = 0;
	int h = 0;

	int workerSize = size -1;
	int receiveBufferSize = 0;
	int recvElem = 0;

	int rows = matrSize;
	int workerRows = 0;
	int workerRowDiff = 0;
	int offset = 0;
	int bufferMatrSize = 0;

	struct timeval start, end;
	long runtime; //runtime in us;
	float runtimeSec; //runtime in sec;
	float flops;

	MPI_Status status; 	//recv status;
	int recvSize;		//number of received entries;

	matr1 = (double*)malloc(matrSize*matrSize*sizeof(double));
	matr2 = (double*)malloc(matrSize*matrSize*sizeof(double));

	receiveBufferSize = matrSize;

	if(workerSize & 0x01 && workerSize > 1){ //odd amount of worker
		receiveBufferSize* = ((matrSize/workerSize)+1);
	}else{
		receiveBufferSize* = (matrSize/workerSize);
	}

	recvBuffer = (double*)malloc(receiveBufferSize*sizeof(double));

	matrErg = (double*)malloc(matrSize*matrSize*sizeof(double));

	if(initStyle == INIT_STYLE_RANDOM){
		initMatrixRandom(matr1, matrSize, TRUE);
		initMatrixRandom(matr2, matrSize, TRUE);
	}else{
		initMatrix(matr1, matrSize, TRUE);	 // matr1[i, j] = i + j;
		initMatrix(matr2, matrSize, FALSE);	 // matr2[i, j] = i * j;
	}

	matr2 = reorderMatrix(matr2, matrSize);

	initMatrixRandom(matrErg, matrSize, FALSE); //matrErg = 0.0;

	#if DEBUG == TRUE
		printf("Matr1\n");
		plot(matr1, matrSize);
		printf("Matr2\n");
		plot(matr2, matrSize);
	#endif

	gettimeofday(&start, NULL);

	//send Matrices to the worker-nodes
	for(i = 1;i < size;i++){
		MPI_Send(matr1, matrSize*matrSize, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
		MPI_Send(matr2, matrSize*matrSize, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
	} 

	//check if there something left to do for the boss
	if(matrSize % workerSize > 0){
		for(i = 1; i <= workerSize; i++){
			if(workerSize & 0x01 && workerSize > 1){ //odd amount of worker
				workerRows = ((matrSize/workerSize)+1);
			}else{
				workerRows = (matrSize/workerSize);
			}

			workerRowDiff = matrSize - i * workerRows;

			if(workerRowDiff < 0){
				workerRows = workerRows + workerRowDiff;
			}
			
			//initial Value for rows = matrSize
			//if rows > 0 => there are work to do
			rows = rows - workerRows;
		}
		
		#if DEBUG == TRUE
		printf(" Boss should calculate %d rows \n", rows);
		#endif

		if(rows > 0){
			offset = (matrSize - rows)*matrSize;
			bufferMatrSize = matrSize*rows;
			bufferMatr = (double*)malloc(bufferMatrSize*sizeof(double));
			for(i = 0;i < rows; i++){//rows of the result-matrix
				for(j = 0; j < matrSize; j++){//colums of the result-matrix
					for(h = 0; h < matrSize; h++){
						*(bufferMatr+j+i*matrSize)+=(*(matr1+h+offset+i*matrSize))*(*(matr2+j*matrSize+h));
					}
				}
			}
		}
	}

	//gather Results from worker-nodes
	for(i = 1; i < size; i++){
		MPI_Recv(recvBuffer, receiveBufferSize, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_DOUBLE, &recvSize);
				
		for(j = 0; j < recvSize; j++){
			*(matrErg+recvElem+j)= *(recvBuffer+j);
		}

		#if DEBUG == TRUE
		printf("Received elements %d current amount of elements %d\n", recvSize, recvElem);
		#endif

		recvElem += recvSize;
	}

	if(rows > 0){
		for(i = 0; i < bufferMatrSize;i++){
			*(matrErg+recvElem+i) = *(bufferMatr+i);
		}
	}	

	gettimeofday(&end, NULL);
	
	#if DEBUG == TRUE
		printf("Result\n");
		plot(matrErg, matrSize);
	#endif

	runtime = calcRuntime(&start, &end);
	runtimeSec = (float)runtime/1e6;
	flops = (2*size*size)/(runtimeSec); //??

	printf("#runtime[us] runtime[sec] flop size\n");
	printf("%ld %f %f %d\n", runtime, runtimeSec, flops, matrSize);
	
	if(matr1 != NULL){	
		#if DEBUG == TRUE
		printf("Release memory for matr1\n");
		#endif	
		free(matr1);
	}

	if(matr2 != NULL){
		#if DEBUG == TRUE
		printf("Release memory for matr2\n");
		#endif
		free(matr2);
	}
	
	if(matrErg != NULL){
		#if DEBUG == TRUE
		printf("Release memory for matrErg\n");
		#endif
		free(matrErg);
	}
	
	if(recvBuffer != NULL){
		#if DEBUG == TRUE
		printf("Release memory for recvBuffer\n");
		#endif 
		free(recvBuffer);
	}

	if(bufferMatr != NULL){
		#if DEBUG == TRUE
			printf("Release memory for bufferMatr\n");
		#endif
		free(bufferMatr);
	}
}

void doWorker(int rank, int size,  int matrSize){
	double* matr1 = NULL;
	double* matr2 = NULL;
	double* erg = NULL;
	int workSize = matrSize;
	int maxRows = matrSize;
	int rows;
	int worker = size - 1;
	int offset = 0;
	int rowdiff = 0;

	int i, j, h = 0;
	
	MPI_Status status;
	
	matr1 = (double*)malloc(matrSize*matrSize*sizeof(double));
	matr2 = (double*)malloc(matrSize*matrSize*sizeof(double));

	if(worker & 0x01 && worker > 1){ //odd amount of worker
		rows=((matrSize/worker)+1);
	}else{
		rows=(matrSize/worker);
	}

	//TODO: Implement Error handling for the case workSize <= 0;
	workSize*=rows;

	offset = (rank -1)*workSize; //workSize is here used as an Helpervalue

	rowdiff = maxRows - rank*rows;
	if(rowdiff < 0){
		rows = rows + rowdiff;
	}
	
	workSize = matrSize*rows;	
	erg = (double*)malloc(workSize*sizeof(double));
	
	MPI_Recv(matr1, SIZE*SIZE, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
	#if DEBUG == TRUE
		printf("%d has received matrix no. 1\n", rank);
	#endif

	MPI_Recv(matr2, SIZE*SIZE, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &status);

	#if DEBUG == TRUE
		printf("%d has received matrix no. 2\n", rank);
	#endif

	#if DEBUG == TRUE
		printf("%d worksize %d rows %d offset %d\n", rank, workSize, rows, offset);
	#endif	

	for(i = 0;i < rows; i++){//rows of the result-matrix
		for(j = 0; j < matrSize; j++){//colums of the result-matrix
			for(h = 0; h < matrSize; h++){
				*(erg+j+i*matrSize)+=(*(matr1+h+offset+i*matrSize))*(*(matr2+j*matrSize+h));
			}
		}
	}

	//Send Result back to boss node
	MPI_Send(erg, workSize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	
	if(matr1 != NULL){
		free(matr1);
	}
	
	if(matr2 != NULL){
		free(matr2);
	}
	
	if(erg != NULL){
		free(erg);
	}
}

/*Function: reorderMatrix() */
double* reorderMatrix(double *matr, unsigned size){
	unsigned i = 0;
	unsigned j = 0;
	unsigned matrSize = size*size;
	double* exchangeMatr = NULL;

	exchangeMatr = (double*)malloc(matrSize*sizeof(double));

	for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			*(exchangeMatr + i*size + j) = *(matr + i + j*size);
		}
	}
	
	free(matr);
	return exchangeMatr;
}

/*Function: plot()*/
void plot(double* mat, int dim){
	int i;
	int j;
	
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
			printf("%lf ", *(mat+j+i*dim));
		}
		printf("\n");
	}	
}
