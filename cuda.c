#include <stdio.h>
#include <stdlib.h>

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

	// Variables
	char line[4];
	long **matrix;
	long ai1, ai2, ai3;

	// Creating matrix
	matrix = (long**) emalloc(3 * sizeof(long*));
	for (int i = 0; i < 3; i++) matrix[i] = (long*) emalloc(3 * sizeof(long));

	fscanf(path_matr, "%[^\n]", line);
	// Filling matrix
	for (int i = 0; i < 3; i++)
		fscanf(path_matr, "%ld %ld %ld", &matrix[i][0], &matrix[i][1], &matrix[i][2])
}

int main(int args, char *argv[]) {
	FILE *path_matr;
	int n_matr; // Number of matrices

	/*Input validation*/
	if (args != 2) {
		fprintf(stderr, "ERROR: Invalid number of arguments.\n");
		exit(1);
	}

	path_matr = fopen(argv[0], "r");
	if (path_matr == NULL) {
		fprintf(stderr, "ERROR: Invalid file to matrices.\n");
		exit(1);
	}

	/*Getting the matrices*/
	fscanf(path_matr, "%d", n_matr);
	for (int i = 0; i < n_matr; i++) {
		file_to_matrix(path_matr, ); // COMPLETAR
	}
}