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
	#define DDBUG 0
#endif

#ifndef OUTPUT
	#define OUTPUT 0
#endif

// Root node
#define ROOT 0

// Inspired by https://stackoverflow.com/a/14517225/24937541
class matrix2D { 
    public:
    std::vector<int> data;
    size_t n;
    size_t nrOfRows;
    size_t nrOfCols;

    // Default constructor (used to declare variables without initializing their value)
    matrix2D() : data(), n(0), nrOfRows(0), nrOfCols(0) {}

    // Constructor square matrix
    matrix2D(size_t dimension) : data(dimension*dimension), n(dimension*dimension), nrOfRows(dimension), nrOfCols(dimension) {}

    // Constructor general matrix
    matrix2D(size_t rows, size_t columns) : data(rows*columns), n(rows*columns), nrOfRows(rows), nrOfCols(columns) {}

    // operator overload for indexing into matrix
    int &operator()(size_t x, size_t y) {
        if (x < 0 || x > nrOfCols || y < 0 || y > nrOfRows ){
            throw std::out_of_range("Index out of range in squareMatrix");
        }
        return data[y*nrOfCols+x];
    }
    
    // To string method printing matrix
    void toString() {
        for (size_t i = 0; i < n; i++){
            if (i % nrOfCols == 0){
                std::cout << std::endl; 
            }
            std::cout << data[i] << "\t"; // Print each value
        }
        std::cout << std::endl << std::endl;
    }
    
    // Get a copy of a row in matrix
    std::vector<int> row(int row_index) {
        std::vector<int> r(nrOfCols);
        r.insert(r.begin(), data.begin() + (row_index * nrOfCols), data.begin() + (row_index * (nrOfCols + 1)));
        return r;
    }

    // Get a copy of a column in matrix
    std::vector<int> column(int column_index) {
        std::vector<int> c(nrOfRows);
        auto itr = data.begin() + column_index;
        for (size_t i = 0; i < nrOfRows; i++){
            c[i] = *itr;
            itr += nrOfCols;
        }
        return c;
    }

    // Resize matrix similar to resize for vectors 
    void resize(size_t rows, size_t columns) {
        nrOfRows = rows;
        nrOfCols = columns;
        n        = rows * columns;
        data.resize(n);
    }
};

/**
 * Distribute all rows from root to the other processes as evenly as
 * possible.
 * @param all_elements Row-vise Matrix to distribute (Not significant in all processes)
 * @param local_elements Row-vise Matrix where local elements will be stored
 * @return global index to first row received 
 */
int distribute_from_root(matrix2D *all_elements, matrix2D *local_elements);

/**
 * Gather elements from all processes onto root. Put root's elements first and
 * thereafter elements from the other nodes in the order of their ranks (so that
 * elements from process i come after the elements from process i-1).
 * @param all_elements Buffer on root where the elements will be stored
 * @param my_elements Elements to be gathered from the current process
 */
void gather_on_root(matrix2D *all_elements, matrix2D *my_elements);

/**
 * Perform the global part of parallel snake sort. When the function returns All 
 * element owned by process i are larger than or equal to all elements owned by 
 * process i-1 and smaller than or equal to all elements owned by process i+1 and
 * elements in process i are sorted in snake order either starting with asending or 
 * desending order depending on first_row_index
 * @param elements Pointer to a matrix of values on the current node. Will point to a matrix with sorted elements when the function returns.
 * @param first_row_index Global index of the first row in local elements matrix used to decide rowsort order.
 */
void global_sort(matrix2D *elements, int first_row_index);

/**
 * Read problem size (dimension of matrix) and generate elements or read problem size and 
 * elements from the file whose name is given as an argument to the function and populate 
 * elements accordingly. Elements will be stored in row-vise order. Nr of elements in
 * file must be able to be evenly constructed into a square matrix. 
 * @param size_or_file_name Dimension of square matrix or name of input file
 * @param elements Pointer to matrix to populate
 * @return Dimension of read matrix
 */
int read_input(char *size_or_file_name, matrix2D *elements);

/**
 * Verify that elements are sorted in asending snake order. If not, write an error
 * message to stdout. Thereafter, print all elements (in the order they are
 * stored) to a file named according to the last argument if output enabled.
 * @param elements Row-vise Matrix to check print
 * @param file_name Name of output file
 * @return 0 on success, -2 on I/O error
 */
int check_and_print(matrix2D *elements, char *file_name);

/**
 * Check if a number of elements are sorted in snake order. If they aren't,
 * print an error message specifying the first two elements that are in wrong
 * order.
 * @param elements Row-vise Matrix to check
 * @return 1 if elements is sorted in ascending order, 0 otherwise
 */
int sorted_snake(matrix2D *elements);

#endif