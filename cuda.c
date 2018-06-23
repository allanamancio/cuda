#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <limits.h> //LONG_MAX
#include <sys/time.h>

#define min(x,y) (y + ((x - y) & ((x - y) >> (sizeof(long) * 8 - 1))))

typedef struct { 
	int row;
	int col;
	long **tab;
} matr;

int SIZE = 3;
int n_matr; // Number of matrices to be reduced
matr *matrices; // Matrices to be reduced
matr min_matr; // Matrix loading the result of the minimuns among the matrices

void print_matrix(matr *m) {
	for (int i = 0; i < m->row; i++)
		for (int j = 0; j < m->col; j++)
			printf("matr[%d][%d] = %ld\n", i, j, m->tab[i][j]);
	printf("\n");
}

void *emalloc(size_t size) {
	void *memory = malloc(size);

	if (!memory) {
		fprintf(stderr, "ERROR: Failed to malloc.\n");
		exit(1);
	}

	return memory;
}

void file_to_matrix(FILE *path_matr, matr *m) {
	/*Reads a file and get the matrix 3x3 from it*/
	// Creating matrix
	long **matrix = (long**) emalloc(SIZE * sizeof(long*));
	for (int i = 0; i < SIZE; i++) matrix[i] = (long*) emalloc(SIZE * sizeof(long));

	char line[20];
	fscanf(path_matr, " %[^\n]", line);
	// Filling matrix
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			fscanf(path_matr, "%ld", &matrix[i][j]);
		}
	}

	m->row = SIZE;
	m->col = SIZE;
	m->tab = matrix;
}

void *reduction(void *raw_position) {
	long min_value = LONG_MAX;
	int position = *((int *) raw_position);
	int i = floor(position/SIZE);
	int j = position - i*SIZE;

	for (int k = 0; k < n_matr; ++k) min_value = min(min_value, matrices[k].tab[i][j]);

	min_matr.tab[i][j] = min_value;
}

int main(int args, char *argv[]) {
	/*Input validation*/
	if (args != 2) {
		fprintf(stderr, "ERROR: Invalid number of arguments.\n");
		exit(1);
	}

	FILE *path_matr = fopen(argv[1], "r");
	if (path_matr == NULL) {
		fprintf(stderr, "ERROR: Invalid file to matrices.\n");
		exit(1);
	}

	/*Getting the matrices*/
	fscanf(path_matr, "%d", &n_matr);
	matrices = emalloc(n_matr * sizeof(matr));

	for (int i = 0; i < n_matr; i++)
		file_to_matrix(path_matr, &matrices[i]);

	min_matr = *((matr *) emalloc(sizeof(matr)));
	min_matr.tab = (long**) emalloc(SIZE * sizeof(long*));
	for (int i = 0; i < SIZE; i++) min_matr.tab[i] = (long*) emalloc(SIZE * sizeof(long));

	printf("Execute the reduction...\n");
	// Timing (beginning)
	struct timeval begin, end;
	gettimeofday(&begin, NULL);

	// Obtaining the minimuns among the matrices
	pthread_t *position_thread = emalloc(SIZE * SIZE * sizeof(pthread_t));
	for (int i = 0; i < SIZE * SIZE; i++) {
		int *raw_pos = emalloc(sizeof(int));
		*raw_pos = i;
		if (pthread_create(&position_thread[i], NULL, reduction, (void *) raw_pos)) {
			fprintf(stderr, "ERROR: thread not created\n");
            exit(1);
		}
	}

	// Closing the threads
	for (int i = 0; i < SIZE * SIZE; i++) {
        if (pthread_join(position_thread[i], NULL)) {
            fprintf(stderr, "ERROR: thread not joined\n");
            exit(1);
        }
    }

    double cpuTime = 1000000*(double)(end.tv_sec - begin.tv_sec);
	cpuTime +=	(double)(end.tv_usec - begin.tv_usec);
	// Timing (ending)
	printf("Execution Time (microseconds): %9.2f\n", cpuTime);

	/*Finishing*/
	print_matrix(&min_matr);

	// Clean up memory and close file
	for (int i=0; i < n_matr; i++)
		free(matrices[i].tab);
	
	free(matrices);
	fclose(path_matr);

	return 0;
}