#ifndef SHEARSORT_H_
#define SHEARSORT_H_

#include <mpi.h>
#include <vector>
#include <array>
#include <algorithm>
#include <math.h>
#include <iostream>

#ifndef DDEBUG
	#define DDBUG 0
#endif

#ifndef OUTPUT
	#define OUTPUT 0
#endif

// Root node
#define ROOT 0



/**
 * Read problem size and elements from the file whose name is given as an
 * argument to the function and populate elements accordingly. Elements will be
 * stored in row-vise order. Nr of elements must be able to be evenly constructed
 * into a square matrix. 
 * @param file_name Name of input file
 * @param elements Pointer to array to be populated
 * @return Number of elements read and stored
 */
int read_input(char *file_name, int *elements);

/**
 * Check if a number of elements are sorted in snake order. If they aren't,
 * print an error message specifying the first two elements that are in wrong
 * order.
 * @param elements Row-vise Matrix Array to check
 * @param n Length of elements
 * @param d width/hight of square matrix 
 * @return 1 if elements is sorted in ascending order, 0 otherwise
 */
int sorted_snake(int *elements, int n, int d);


#endif