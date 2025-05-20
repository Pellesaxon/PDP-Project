#ifndef SHEARSORT_H_
#define SHEARSORT_H_

#include <mpi.h>
#include <vector>
#include <array>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>

#ifndef DDEBUG
	#define DDBUG 1
#endif

#ifndef OUTPUT
	#define OUTPUT 1
#endif

// Root node
#define ROOT 0

// Inspired by https://stackoverflow.com/a/14517225/24937541
class squareMatrix { 
    std::vector<int> data;
public:
    size_t n;
    size_t d;

    squareMatrix() : data(), n(0), d(0) {}
    squareMatrix(size_t dimension) : data(dimension*dimension), n(dimension*dimension), d(dimension) {}

    int &operator()(size_t x, size_t y) {
        if (x < 0 || x > d || y < 0 || y > d ){
            throw std::out_of_range ("Index out of range in squareMatrix");
        }
        return data[y*d+x];
    }
};


/**
 * Read problem size (dimension of matrix) and elements from the file whose name is given as an
 * argument to the function and populate elements accordingly. Elements will be
 * stored in row-vise order. Nr of elements must be able to be evenly constructed
 * into a square matrix. 
 * @param file_name Name of input file
 * @return Populated matrix
 */
squareMatrix read_input(char *file_name);

/**
 * Verify that elements are sorted in asending snake order. If not, write an error
 * message to stdout. Thereafter, print all elements (in the order they are
 * stored) to a file named according to the last argument.
 * @param elements Elements to check and print
 * @param n Number of elements
 * @param d width/hight of square matrix 
 * @param file_name Name of output file
 * @return 0 on success, -2 on I/O error
 */
int check_and_print(squareMatrix *elements, char *file_name);

/**
 * Check if a number of elements are sorted in snake order. If they aren't,
 * print an error message specifying the first two elements that are in wrong
 * order.
 * @param elements Row-vise Matrix Array to check
 * @param n Length of elements
 * @param d width/hight of square matrix 
 * @return 1 if elements is sorted in ascending order, 0 otherwise
 */
int sorted_snake(squareMatrix *elements);


#endif