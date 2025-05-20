#include "shearSort.h"


int main(int argc, char **argv) {
    int num_proc, myid;

    if (3 != argc) {
		printf("Usage: input_file output_file \n Amount of arguments submitted: %d\n", argc);
		return 1;
	}

    char *input_name = argv[1];
	char *output_name = argv[2];

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    squareMatrix matrix_A;

    if (myid == ROOT){
        if (DDBUG){
            std::cout << myid << ": " << "Reading input-file" << std::endl; // Print each word
        }
        matrix_A = read_input(input_name);
        
        if (matrix_A.d <= 0){
            printf("Unable to read input file or dimension < 1\n");
            return 1;
        }
    }

    
    


    if (myid == ROOT){
        if (DDBUG){
            std::cout << myid << ": " << "Checking output and writing output" << std::endl; // Print each word
        }
        check_and_print(&matrix_A, output_name);
    } 

    MPI_Finalize();
    return 0;

}



// Based on functions from previous assignments
squareMatrix read_input(char *file_name) {
    std::ifstream inputFile;
    inputFile.open(file_name);

    if (!inputFile.is_open()) {
        perror("Couldn't open input file");
		return -2;
    }

    int dimensions;

    inputFile >> dimensions;
    if (inputFile.fail()){
        perror("Couldn't read square matrix size from input file");
        return -2;
    }

    if (DDBUG){
        std::cout << "Dim = " << dimensions << std::endl; // Print each word
    }

    squareMatrix elements = squareMatrix(dimensions);
    
    int value;
    int x = 0, y = 0;

    while (inputFile >> value) {
        if (x >= dimensions){
            if (DDBUG){
                std::cout << std::endl; 
            }
            x = 0;
            y++;
        }

        if (DDBUG){
            // std::cout << value << " at location: " << x << ", " << y << "\t"; // Print each value
            std::cout << value << "\t"; // Print each value
        }

        elements(x,y) = value;
        x++;
    }

    if (DDBUG){
        std::cout << std::endl; 
    }

    inputFile.close();

	return elements;
}

// Based on functions from previous assignments
int check_and_print(squareMatrix *elements, char *file_name) {
	sorted_snake(elements);
	
	// FILE *file;
	// if (NULL == (file = fopen(file_name, "w"))) {
	// 	perror("Couldn't open output file");
	// 	return -2;
	// }
	// for (int i = 0; i < n; i++) {
	// 	if (0 > fprintf(file, "%d ", elements[i])) {
	// 		perror("Couldn't write to output file");
	// 		return -2;
	// 	}
	// }
	// if (0 > fprintf(file, "\n")) {
	// 	perror("Couldn't write to output file");
	// 	return -2;
	// }
	// if (0 != fclose(file)) {
	// 	perror("Warning: couldn't close output file");
	// 	return -2;
	// }
	return 0;
}

// Based on functions from previous assignments
int sorted_snake(squareMatrix *elements){
    int dim = elements->d;
    int prev = 0;
    
    if (DDBUG){
        std::cout << "Checking sorted: " << std::endl; // Print each word
    }
    
    int x = 0, y = 0;
    for (y = 0; y < dim; y++){
		if (y%2 == 0){
            for (x = 0; x < dim; x++){
                if (DDBUG){
                    std::cout << prev << " and " << (*elements)(x,y) << std::endl; 
                }
                if (!(prev <= (*elements)(x,y))){
                    std::cerr << "Warning: Elements not sorted: " << prev << " and " << (*elements)(x,y) << std::endl;
                    return 0;
                }
                prev = (*elements)(x,y);
            }
        }
        else {
            for (x = dim-1; x >= 0; x--){
                if (DDBUG){
                    std::cout << prev << " and " << (*elements)(x,y) << std::endl; 
                }
                if (!(prev <= (*elements)(x,y))){
                    std::cerr << "Warning: Elements not sorted: " << prev << " and " << (*elements)(x,y) << std::endl;
                    return 0;
                }
                prev = (*elements)(x,y);
            }
        }
	}

	return 1;
}