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

    // TODO: Divide rows and colls evenly 
    // TODO: Setup local data
    int dim = matrix_A.d;
    int col_steps = ceil(log2(dim));
    int row_steps = col_steps + 1; 

    for (int i = 0; i < row_steps; i++){
        for (int row = 0; row < dim; row++){
            if (DDBUG){
                std::cout << myid << ": " << "Before with row sort" << std::endl;
            }
            if (row % 2 == 0){
                if (DDBUG){
                    std::cout << myid << ": " << "Sorting even row" << std::endl;
                }
                std::sort(matrix_A.data.begin() + matrix_A.d * row, matrix_A.data.begin() + matrix_A.d * (row+1));
            }
            else {
                if (DDBUG){
                    std::cout << myid << ": " << "Sorting odd row" << std::endl;
                }
                std::sort(matrix_A.data.begin() + matrix_A.d * row, matrix_A.data.begin() + matrix_A.d * (row+1), std::greater<int>());
            }
        }

        if (DDBUG){
            std::cout << myid << ": " << "Done with row sort" << std::endl; 
        }

        if (i < col_steps){
            for (int col = 0; col < dim; col++){
                std::vector <int> column = matrix_A.column(col);
                std::sort(column.begin(), column.end());
                
                auto itr = column.begin();
                for (int y = 0; y < dim; y++){
                    matrix_A(col, y) = *itr;
                    itr++;
                }
            }
        }

        if (DDBUG){
            std::cout << myid << ": " << "Done with column sort" << std::endl; 
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

    if (OUTPUT){
        std::ofstream outputfile;
        outputfile.open(file_name);

        if (!outputfile.is_open()) {
            perror("Couldn't open output file");
            return -2;
        }

        int i = 0;
        for (int value : elements->data){
            outputfile << std::to_string(value) << " ";
            if ((i+1) % elements->d == 0){
                outputfile << std::endl;
            }
            i++;
        }

        outputfile.close();
    }
    else {
        std::cerr << "Warning: Output disables" << std::endl;
    }
	return 0;
}

// Based on functions from previous assignments
int sorted_snake(squareMatrix *elements){
    int dim = elements->d;
    int prev = 0;
    
    if (DDBUG){
        elements->toString();
        std::cout << "Checking sorted " << std::endl; // Print each word
    }
    
    // Loop over all rows and check each column
    for (int y = 0; y < dim; y++){
        // Checking ascending order
		if (y%2 == 0){
            for (int x = 0; x < dim; x++){
                // if (DDBUG){
                //     std::cout << prev << " and " << (*elements)(x,y) << std::endl; 
                // }
                if (!(prev <= (*elements)(x,y))){
                    std::cerr << "Warning: Elements not sorted: " << prev << " and " << (*elements)(x,y) << std::endl;
                    return 0;
                }
                prev = (*elements)(x,y);
            }
        }
        // Checking descending order
        else {
            for (int x = dim-1; x >= 0; x--){
                // if (DDBUG){
                //     std::cout << prev << " and " << (*elements)(x,y) << std::endl; 
                // }
                if (!(prev <= (*elements)(x,y))){
                    std::cerr << "Warning: Elements " << prev << " and " << (*elements)(x,y) << " not sorted" << std::endl;
                    return 0;
                }
                prev = (*elements)(x,y);
            }
        }
	}

    if (DDBUG){
        std::cout << "Matrix sorted correctly" << std::endl; // Print each word
    }

	return 1;
}