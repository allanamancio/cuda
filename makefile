main:
	nvcc cuda.cu -o main

cpu:
	gcc cuda.c -std=c11 -pthread -lm -o cpu
