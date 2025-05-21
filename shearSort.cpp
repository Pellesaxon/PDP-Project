#include "shearSort.h"


int main(int argc, char **argv) {
    int num_proc, myid, dim;

    if (3 != argc) {
		printf("Usage: input_file output_file \n Amount of arguments submitted: %d\n", argc);
		return 1;
	}

    // Read cmdline arguments
    char *input_name = argv[1];
	char *output_name = argv[2];

    // Setup MPI
    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    matrix2D matrix_A;
    // std::vector<int> rowDistribution;

    if (myid == ROOT){
        if (DDBUG){
            std::cout << myid << ": " << "Reading input-file" << std::endl; // Print each word
        }
        dim = read_input(input_name, &matrix_A);
        
        if (dim <= 0){
            printf("Unable to read input file or dimension < 1\n");
            return 1;
        }
    }

    // Split up the elements on all processes
    matrix2D local_matrix;
    int first_row_index = distribute_from_root(&matrix_A, &local_matrix);


    dim = matrix_A.nrOfCols;
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
                std::sort(matrix_A.data.begin() + matrix_A.nrOfCols * row, matrix_A.data.begin() + matrix_A.nrOfCols * (row+1));
            }
            else {
                if (DDBUG){
                    std::cout << myid << ": " << "Sorting odd row" << std::endl;
                }
                std::sort(matrix_A.data.begin() + matrix_A.nrOfCols * row, matrix_A.data.begin() + matrix_A.nrOfCols * (row+1), std::greater<int>());
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

int distribute_from_root(matrix2D *all_elements, matrix2D *local_elemets){
	int num_proc, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    // Struct for mpi datatype to send all local data in one scatter call
    struct LocalMatrixInfo{
        int local_row_count; 
        int column_count;
        int local_row_index_start; 
    } local_info;
    
    // MPI datatype that match LocalMatrixInfo
    MPI_Datatype local_info_type;
    MPI_Type_contiguous(3 , MPI_INT , &local_info_type);
    MPI_Type_commit(&local_info_type);

    // Init buffers used by ROOT 
    std::vector<LocalMatrixInfo> local_info_array;
    std::vector<int> sendcounts;
    std::vector<int> displs;

    if (myid == ROOT) {
        // Resize to avoid allocation of memory in all processes
        local_info_array.resize(num_proc);
        sendcounts.resize (num_proc);
        displs.resize(num_proc);

        // Shall be equal as long as matrix is square
        int nrOfRows = all_elements->nrOfRows;
        int nrOfCols = all_elements->nrOfCols;

        // Initialize first process info
        local_info_array[0].local_row_count  = nrOfRows / num_proc + (0 < nrOfRows % num_proc ? 1 : 0);
        local_info_array[0].column_count = nrOfCols;
        local_info_array[0].local_row_index_start = 0;

        sendcounts[0] = local_info_array[0].local_row_count * nrOfCols;
        displs[0] = 0;

        // initilize the rest based on previous entries
		for (int i = 1; i < num_proc; ++i) {
            // Information to scatter
            local_info_array[i].local_row_count  = nrOfRows / num_proc + (i < nrOfRows % num_proc ? 1 : 0);
            local_info_array[i].column_count = nrOfCols;
            local_info_array[i].local_row_index_start = local_info_array[i-1].local_row_index_start + local_info_array[i-1].local_row_count;

            // ROOT LOCAL SEND INFORMATION 
            sendcounts[i] = local_info_array[i].local_row_count * nrOfCols;
            displs[i] = displs[i - 1] + sendcounts[i - 1];
		}
	}

	if (DDBUG) {
		printf("PROCESS %d Before LocalMatrixInfo scatter \n", myid);
	}

    // Scatter the local info to each process
	MPI_Scatter(local_info_array.data(), 1, local_info_type, &local_info, 1, local_info_type, ROOT, MPI_COMM_WORLD);

    // Use local info to resize receive matrix
    local_elemets->resize(local_info.local_row_count, local_info.column_count);

	if (DDBUG) {
		printf("PROCESS %d Before scatterv \n", myid);
	}

    // Scatter rows to all processes
	MPI_Scatterv(all_elements->data.data(), sendcounts.data(), displs.data(), MPI_INT,
                 local_elemets->data.data(), local_elemets->n, MPI_INT,
				 0, MPI_COMM_WORLD);
	
    MPI_Type_free(&local_info_type);

	return (local_info.local_row_index_start);
}


// Based on functions from previous assignments
int read_input(char *file_name, matrix2D *elements){
    std::ifstream inputFile;
    inputFile.open(file_name);

    if (!inputFile.is_open()) {
        perror("Couldn't open input file");
		return -2;
    }

    int dimension;

    inputFile >> dimension;
    if (inputFile.fail()){
        perror("Couldn't read square matrix size from input file");
        return -2;
    }

    if (DDBUG){
        std::cout << "Dim = " << dimension << std::endl; // Print each word
    }

    elements->resize(dimension, dimension);

    int x = 0, y = 0;
    size_t readcount = 0;
    while (inputFile >> (*elements)(x,y)) {
        if (DDBUG){
            // std::cout << value << " at location: " << x << ", " << y << "\t"; // Print each value
            std::cout << (*elements)(x,y) << "\t"; // Print each value
        }
        
        x++;
        readcount++;
        if (x >= dimension){
            x = 0;
            y++;

            if (DDBUG){
                std::cout << std::endl; 
            }
        }
    }

    if (DDBUG){
        std::cout << std::endl; 
    }

    inputFile.close();

    if (DDBUG){
        // std::cout << value << " at location: " << x << ", " << y << "\t"; // Print each value
        std::cout << "readcount = "<< readcount << " and n = " << elements->n << std::endl; // Print each value
    }

    if (!(readcount == elements->n)){
        perror("Too few arguments to fill matrix");
        return -2;
    }

	return dimension;
}

// Based on functions from previous assignments
int check_and_print(matrix2D *elements, char *file_name) {
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
            if ((i+1) % elements->nrOfCols == 0){
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
int sorted_snake(matrix2D *elements){
    int dim = elements->nrOfCols;
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