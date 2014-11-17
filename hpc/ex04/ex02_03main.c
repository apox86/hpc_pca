#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0

#define DEBUG FALSE

#define SIZE 2048

#define INIT_STYLE_SIMPLE 0
#define INIT_STYLE_RANDOM 1

long calcRuntime(struct timeval *begin, struct timeval *end);

double drand();

void initMatrixRandom(double* matr,int dim,int random);
void initMatrix(double* matr,int dim,int multiply);

void doBoss(int size, int matrSize, int initStyle);
void doWorker(int rank,int size, int matrSize);

void plot(double* mat,int dim);

int main(int argc,char** argv){

	int rank,size;

	int matrSize = SIZE;
	int initStyle = INIT_STYLE_RANDOM;

	MPI_Init(&argc,&argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);

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

	if(rank == 0){
		//boss node;
		doBoss(size,matrSize,initStyle);
	}else{
		//worker node;
		doWorker(rank,size,matrSize);
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
void initMatrixRandom(double* matr,int dim,int random){
	int i;
	int j;
	
	for(i = 0; i <dim; i++){
		for(j = 0; j < dim; j++){
			if(random == TRUE){
				*(matr+j+i*dim)=drand();	
			}else{
				*(matr+j+i*dim)=0.0;
			}
		}
	}
}

/*Function: initMatrix()*/
void initMatrix(double* matr,int dim,int multiply){
	int i;
	int j;
	
	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			if(multiply == TRUE){
				*(matr+j+i*dim)=i*j;	
			}else{
				*(matr+j+i*dim)=i+j;
			}
		}
	}

}

void doBoss(int size, int matrSize, int initStyle){
	double* matr1 = NULL;
	double* matr2 = NULL;
	double* recvBuffer = NULL;
	double* matrErg = NULL;
	
	int i = 1;
	int j = 0;
	int workerSize = size -1;
	int receiveBufferSize = 0;
	int recvElem = 0;
	
	struct timeval start,end;
	long runtime; //runtime in us;
	float runtimeSec; //runtime in sec;
	float flops;

	MPI_Status status; 	//recv status;
	int recvSize;		//number of received entries;
	
	
	matr1 = (double*)malloc(matrSize*matrSize*sizeof(double));
	matr2 = (double*)malloc(matrSize*matrSize*sizeof(double));
	
	receiveBufferSize = matrSize;
	
	if(workerSize & 0x01 && workerSize > 1){ //odd amount of worker
		receiveBufferSize*=((matrSize/workerSize)+1);
	}else{
		receiveBufferSize*=(matrSize/workerSize);
	}

	recvBuffer = (double*)malloc(receiveBufferSize*sizeof(double));

	matrErg = (double*)malloc(matrSize*matrSize*sizeof(double));

	if(initStyle == INIT_STYLE_RANDOM){
		initMatrixRandom(matr1,matrSize,TRUE);
		initMatrixRandom(matr2,matrSize,TRUE);
	}else{
		initMatrix(matr1,matrSize,TRUE);	 // matr1[i,j] = i + j;
		initMatrix(matr2,matrSize,FALSE);	 // matr2[i,j] = i * j;
	}

	initMatrixRandom(matrErg,matrSize,FALSE); //matrErg = 0.0;

	#if DEBUG == TRUE
		printf("Matr1\n");
		plot(matr1,matrSize);
		printf("Matr2\n");
		plot(matr2,matrSize);
	#endif

	gettimeofday(&start,NULL);

	//send Matrices to the worker-nodes
	for(i = 1;i < size;i++){
		MPI_Send(matr1, matrSize*matrSize, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
		MPI_Send(matr2, matrSize*matrSize, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
	} 

	//gather Results from worker-nodes
	for(i = 1; i < size; i++){
		MPI_Recv(recvBuffer,receiveBufferSize,MPI_DOUBLE,i,0,MPI_COMM_WORLD,&status);
		MPI_Get_count(&status,MPI_DOUBLE,&recvSize);
				
		for(j = 0; j < recvSize; j++){
			*(matrErg+recvElem+j)= *(recvBuffer+j);
		}

		#if DEBUG == TRUE
		printf("Received elements %d current amount of elements %d\n",recvSize,recvElem);
		#endif

		recvElem += recvSize;
	}
	
	gettimeofday(&end,NULL);
	
	#if DEBUG == TRUE
		printf("Result\n");
		plot(matrErg,SIZE);
	#endif

	runtime = calcRuntime(&start,&end);
	runtimeSec = (float)runtime/1e6;
	flops = (2*size*size)/(runtimeSec); //??

	printf("#runtime[us] runtime[sec] flop size\n");
	printf("%ld %f %f %d\n",runtime,runtimeSec,flops,SIZE);
	
	free(matr1);
	free(matr2);
	free(matrErg);
	free(recvBuffer);
	
}

void doWorker(int rank,int size, int matrSize){
	double* matr1 = NULL;
	double* matr2 = NULL;
	double* erg = NULL;
	int workSize = matrSize;
	int maxRows = matrSize;
	int rows;
	int worker = size - 1;
	int offset = 0;
	int rowdiff = 0;

	int i,j,h = 0;
	
	MPI_Status status;
	
	matr1 = (double*)malloc(SIZE*SIZE*sizeof(double));
	matr2 = (double*)malloc(SIZE*SIZE*sizeof(double));

	if(worker & 0x01 && worker > 1){ //odd amount of worker
		rows=((SIZE/worker)+1);
	}else{
		rows=(SIZE/worker);
	}
	
	rowdiff = maxRows - rank*rows;
	if(rowdiff < 0){
		rows = rows - rowdiff;
	}

	//TODO: Implement Error handling for the case workSize <= 0;
	workSize*=rows;
	erg = (double*)malloc(workSize*sizeof(double));
	
	MPI_Recv(matr1,SIZE*SIZE,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&status);
	#if DEBUG == TRUE
		printf("%d has received matrix no. 1\n",rank);
	#endif

	MPI_Recv(matr2,SIZE*SIZE,MPI_DOUBLE,0,2,MPI_COMM_WORLD,&status);

	#if DEBUG == TRUE
		printf("%d has received matrix no. 2\n",rank);
	#endif
	
	//do work (calculate)
	offset = (rank-1)*workSize;
	#if DEBUG == TRUE
		printf("%d offset %d\n",rank,offset);
	#endif	

	for(i = 0;i < rows; i++){//rows of the result-matrix
		for(j = 0; j < matrSize; j++){//colums of the result-matrix
			for(h = 0; h < matrSize; h++){
				*(erg+j+i*matrSize)+=(*(matr1+h+offset+i*matrSize))*(*(matr2+j+h*matrSize));
			}
		}
	}

	//Send Result back to boss node
	MPI_Send(erg,workSize,MPI_DOUBLE,0,0,MPI_COMM_WORLD);

	free(matr1);
	free(matr2);
	free(erg);
}

/*Function: plot()*/
void plot(double* mat,int dim){
	int i;
	int j;
	
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
			printf("%lf ",*(mat+j+i*dim));
		}
		printf("\n");
	}	
}
