#include <stdio.h>
#include <stdlib.h>

typedef struct { 
	int row;
	int col;
	long **tab;
} matr;

int SIZE = 3;

//DEBUG
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
	int n_matr; // Number of matrices
	fscanf(path_matr, "%d", &n_matr);
	matr *matrices = emalloc(n_matr * sizeof(matr));

	for (int i = 0; i < n_matr; i++) {
		file_to_matrix(path_matr, &matrices[i]); // COMPLETAR
		print_matrix(&matrices[i]);
	}

	for (int i=0; i < n_matr; i++)
		free(matrices[i].tab);
	
	free(matrices);
	fclose(path_matr);

	return 0;
}