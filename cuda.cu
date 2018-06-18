#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cuda_profiler_api.h>
#include <assert.h>

#define min(x,y) (y + ((x - y) & ((x - y) >> (sizeof(long) * 8 - 1))))

const int Tile_Width = 1;
const int WIDTH = 3;

void print_matrix(long *m) {
	for (int i = 0; i < WIDTH; i++)
		for (int j = 0; j < WIDTH; j++)
			printf("P[%d][%d] = %ld\n", i, j, m[i * WIDTH + j]);
	printf("\n");
}

// Convenience function for checking CUDA runtime API results
// can be wrapped around any runtime API call. No-op in release builds.
inline cudaError_t checkCuda(cudaError_t result) {
#if defined(DEBUG) || defined(_DEBUG)
	if (result != cudaSuccess) {
		fprintf(stderr, "CUDA Runtime Error: %s\n", cudaGetErrorString(result));
		assert(result == cudaSuccess);
	}
#endif
	return result;
}


__global__ void reduction(long* Pd, long* Nd, int ndsize) {
	long Pvalue = LONG_MAX;

	int i = blockIdx.y * Tile_Width + threadIdx.y;
	int j = blockIdx.x * Tile_Width + threadIdx.x;

	for (int k = 0; k < ndsize; ++k) {
		Pvalue = min(Pvalue, Nd[k * WIDTH * WIDTH + i * WIDTH + j]);
	}

	Pd[i * WIDTH + j] = Pvalue;
}

void *emalloc(size_t size) {
	void *memory = malloc(size);

	if (!memory) {
		fprintf(stderr, "ERROR: Failed to malloc.\n");
		exit(1);
	}

	return memory;
}


void file_to_matrix(FILE *path_matr, long* m, int ind) {
	/*Reads a file and get the matrix 3x3 from it*/
	// Creating matrix
	char line[20];
	fscanf(path_matr, " %[^\n]", line);
	// Filling matrix
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < WIDTH; j++) {
			fscanf(path_matr, "%ld", &m[(ind * WIDTH * WIDTH) + i * WIDTH + j]);
		}
	}
}


int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Syntax: %s <matrix file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE *path_matr = fopen(argv[1], "r");
	if (path_matr == NULL) {
		fprintf(stderr, "ERROR: Invalid file to matrices.\n");
		exit(1);
	}

	int n_matr; // Number of matrices
	fscanf(path_matr, "%d", &n_matr);


	long* M = (long*) emalloc(n_matr * WIDTH * WIDTH * sizeof(long*));
	long* P = (long*) emalloc(WIDTH * WIDTH * sizeof(long));
 
	for (int i = 0; i < n_matr; i++) {
		file_to_matrix(path_matr, M, i);
	}

	checkCuda( cudaSetDevice(0) );
	cudaDeviceReset();

	// allocate device matrices (linearized)
	//printf("Allocate device matrices (linearized)...\n");
	long* Nd = NULL;
	long* Pd = NULL;

	checkCuda( cudaMalloc((void**) &Nd, n_matr * WIDTH * WIDTH * sizeof(long)) );
	checkCuda( cudaMalloc((void**) &Pd, WIDTH * WIDTH * sizeof(long)) );

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	// copy host memory to device

	// cudaMemcpy -> faz copias de vetores do host para o device
	checkCuda( cudaMemcpy(Nd, M, n_matr * WIDTH * WIDTH * sizeof(long), cudaMemcpyHostToDevice) );
	checkCuda( cudaMemcpy(Pd, P, WIDTH * WIDTH * sizeof(long), cudaMemcpyHostToDevice) );
	// execute the kernel
	printf("Execute the kernel...\n");


	int GridSize = (WIDTH + Tile_Width-1) / Tile_Width;
	dim3 gridDim(GridSize, GridSize);
	dim3 blockDim(Tile_Width, Tile_Width);

	cudaProfilerStart();
	reduction<<< gridDim, blockDim >>>(Pd, Nd, n_matr);
	cudaProfilerStop();

	// copy result from device to host
	checkCuda( cudaMemcpy( P, Pd, WIDTH * WIDTH * sizeof(long),cudaMemcpyDeviceToHost) );
	gettimeofday(&end, NULL);

	double gpuTime = 1000000*(double)(end.tv_sec - begin.tv_sec);
	gpuTime +=	(double)(end.tv_usec - begin.tv_usec);

	// print times
	printf("Execution Time (microseconds): %9.2f\n", gpuTime);

	print_matrix(P);
	/* print result
	FILE *ptr_file;
	ptr_file =fopen("matMul_gpu_globalmem.out", "w");
	if (!ptr_file) return 1;

	for (int i=0; i < Width; i++){
			for (int j=0; j < Width; j++) fprintf(ptr_file,"%6.2f ", P[i * Width + j]);
			fprintf(ptr_file,"\n");
	}
	fclose(ptr_file); */


	// clean up memory
	free(M);
	free(P);
	checkCuda( cudaFree(Nd) );
	checkCuda( cudaFree(Pd) );

	return 0;
}