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
    public:
    std::vector<int> data;
    size_t n;
    size_t d;

    // Default constructor (used to declare variables without initializing their value)
    squareMatrix() : data(), n(0), d(0) {}

    // Constructor 
    squareMatrix(size_t dimension) : data(dimension*dimension), n(dimension*dimension), d(dimension) {}

    // operator overload for indexing into matrix
    int &operator()(size_t x, size_t y) {
        if (x < 0 || x > d || y < 0 || y > d ){
            throw std::out_of_range ("Index out of range in squareMatrix");
        }
        return data[y*d+x];
    }
    
    // To string method printing matrix
    void toString() {
        for (size_t i = 0; i < n; i++){
            if (i % d == 0){
                std::cout << std::endl; 
            }
            std::cout << data[i] << "\t"; // Print each value
        }
        std::cout << std::endl;
    }
    
    // Get a copy of a row in matrix
    std::vector<int> row(int row_index) {
        std::vector<int> r(d);
        r.insert(r.begin(), data.begin() + (row_index * d), data.begin() + (row_index * (d + 1)));

        return r;
    }

    // Get a copy of a column in matrix
    std::vector<int> column(int column_index) {
        std::vector<int> c(d);
        
        auto itr = data.begin() + column_index;
        for (size_t i = 0; i < d; i++){
            c[i] = *itr;
            itr += d;
        }
        return c;
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
 * @param elements Row-vise Matrix to check print
 * @param file_name Name of output file
 * @return 0 on success, -2 on I/O error
 */
int check_and_print(squareMatrix *elements, char *file_name);

/**
 * Check if a number of elements are sorted in snake order. If they aren't,
 * print an error message specifying the first two elements that are in wrong
 * order.
 * @param elements Row-vise Matrix to check
 * @return 1 if elements is sorted in ascending order, 0 otherwise
 */
int sorted_snake(squareMatrix *elements);


#endif