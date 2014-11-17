#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define DEBUG 0

#define TRUE 1
#define FALSE 0

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

double drand(){
	return (rand() /(double) RAND_MAX) * 10.;
}

double initMatr(double* matr, unsigned N, int zeros){
	unsigned i = 0;
	unsigned j = 0;
	
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			if (zeros == TRUE){
				*(matr + i*N + j) = 0;
			}
			else{
				*(matr + i*N + j) = drand();
			}
		}
	}
}


double* reorderMatrix(double *matr,unsigned size){
	unsigned i = 0;
	unsigned j = 0;
	unsigned matrSize = size*size;
	double* exchangeMatr = NULL;

	exchangeMatr = (double*)malloc(matrSize*sizeof(double));;

	for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			*(exchangeMatr + i*size + j) = *(matr + i + j*size);
		}
	}

	/*for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			*(matr + i*size + j) = *(exchangeMatr + i*size + j);
		}
	}

	free(exchangeMatr);*/
	
	free(matr);
	return exchangeMatr;
}

void plotMatr(double * matr, unsigned N){
	unsigned i = 0;
	unsigned j = 0;
	
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			printf("%lf ", *(matr+i*N+j));
		}
		printf("\n");
	}
	printf("\n");
}

void multMatr(double* a, double* b, double *c, unsigned N){
	unsigned i = 0;
	unsigned j = 0;
	unsigned k = 0;

	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			for (k = 0; k < N; k++){
				*(c + i*N + j) += (*(a + j*N + k))*(*(b + k*N + j));
			}
		}
	}
}

void multMatr_opt(double* a, double* b, double *c, unsigned N){
	unsigned i = 0;
	unsigned j = 0;
	unsigned k = 0;

	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			for (k = 0; k < N; k++){
				*(c + i*N + j) += (*(a + j*N + k))*(*(b + j*N + k));
			}
		}
	}
}

int main(int argc, char** argv){
	unsigned size = 2048;
	unsigned matrSize = size*size;
	long elapsedTime = 0;
	struct timeval start, end, start2;
	double* matrA, *matrB, *matrC;

	if (argc == 2){
		size = atoi(argv[1]);
	}

	printf("Size %d\n",size);

	matrA = (double*)malloc(matrSize*sizeof(double));
	matrB = (double*)malloc(matrSize*sizeof(double));
	matrC = (double*)malloc(matrSize*sizeof(double));

	initMatr(matrA, size, FALSE);
	initMatr(matrB, size, FALSE);
	initMatr(matrC, size, TRUE);

	#if DEBUG == 1
	printf("matrA\n");
	plotMatr(matrA, size);

	printf("matrB\n");
	plotMatr(matrB, size);
	#endif

	gettimeofday(&start,NULL);
	multMatr(matrA, matrB, matrC, size);
	gettimeofday(&end,NULL);

	#if DEBUG == 1
	printf("matrC\n");
	plotMatr(matrC, size);
	#endif

	elapsedTime = calcRuntime(&start,&end);
	printf("Time elapsed %ld us\n\n",elapsedTime);


	gettimeofday(&start,NULL);
	matrB = reorderMatrix(matrB,size);

	gettimeofday(&start2,NULL);
	multMatr_opt(matrA, matrB, matrC, size);
	gettimeofday(&end,NULL);

	//elapsedTime = calcRuntime(&start2,&end);
	//printf("Time elapsed %ld us (without reorder)\n", elapsedTime);
	
	elapsedTime = calcRuntime(&start,&end);
	printf("Time elapsed %ld us \n", elapsedTime);

	free(matrA);
	free(matrB);
	free(matrC);
}
