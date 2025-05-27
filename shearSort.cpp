#include "shearSort.h"


int main(int argc, char **argv) {
    int num_proc, myid, dim;
    double start, execution_time;

    if (3 != argc) {
		printf("Usage: {input_file | square dim} output_file \n Amount of arguments submitted: %d\n", argc);
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
            std::cout << myid << ": " << "Reading input-file" << std::endl;
        }
        dim = read_input(input_name, &matrix_A);
        
        if (dim <= 0){
            printf("Unable to read input file or dimension < 1\n");
            return 1;
        }
    }
    
    // Start timer
    start = MPI_Wtime();

    // Split up the elements on all processes
    matrix2D local_matrix;
    int first_row_index = distribute_from_root(&matrix_A, &local_matrix);

    if (DDBUG){
        if (myid==ROOT){
            std::cout << myid << ": " << "Printing local_matrix" << std::endl;
            local_matrix.toString();
        }
    }


    global_sort(&local_matrix, first_row_index);

    gather_on_root(&matrix_A, &local_matrix);
    

    if (myid == ROOT){
        // Stop timer
        execution_time = MPI_Wtime() - start;

        if (DDBUG){
            std::cout << myid << ": " << "Checking output and writing output" << std::endl; // Print each word
        }
        check_and_print(&matrix_A, output_name);
        std::cout << execution_time << std::endl;
    } 

    MPI_Finalize();
    return 0;

}



// Based on functions from previous assignments
int read_input(char *file_name, matrix2D *elements){
    int dimension;

    // if filename is a number then create a matrix instead of reading it
    try 
    {
        std::size_t pos{};
        dimension = std::stoi(file_name, &pos);

        // allocate space for data
        elements->resize(dimension, dimension);

        // fill matrix with random elements
        int x = 0, y = 0;
        while (x < dimension && y < dimension) {
            (*elements)(x,y) = std::rand();
            if (DDBUG){
                std::cout << (*elements)(x,y) << "\t"; // Print each value
            }
            x++;
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
            std::cout << "n = " << elements->n << std::endl; 
        }

        return dimension;
    }
    
    // Cath if it is not a number and read file try to instead
    catch (std::invalid_argument const& ex) 
    {   
        if (DDBUG){
            std::cout << "Input is not a number: " << file_name << std::endl;
            std::cout << "Reading inputfile" << std::endl;
        }
    }
    

    // read file based on earlier assignments
    std::ifstream inputFile;
    inputFile.open(file_name);

    if (!inputFile.is_open()) {
        perror("Couldn't open input file");
		return -2;
    }

    inputFile >> dimension;
    if (inputFile.fail()){
        perror("Couldn't read square matrix size from input file");
        return -2;
    }

    if (DDBUG){
        std::cout << "Dim = " << dimension << std::endl; // Print each word
    }

    // allocate space for data
    elements->resize(dimension, dimension);

    // fill matrix with elements from inputfile
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

int distribute_from_root(matrix2D *all_elements, matrix2D *local_elements){
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
    local_elements->resize(local_info.local_row_count, local_info.column_count);

	if (DDBUG) {
		printf("PROCESS %d Before scatterv \n", myid);
	}

    // Scatter rows to all processes
	MPI_Scatterv(all_elements->data.data(), sendcounts.data(), displs.data(), MPI_INT,
                 local_elements->data.data(), local_elements->n, MPI_INT,
				 ROOT, MPI_COMM_WORLD);
	
    MPI_Type_free(&local_info_type);

	return (local_info.local_row_index_start);
}


void global_sort(matrix2D *elements, int first_row_index) {
    int num_proc, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    int global_dim = elements->nrOfCols;
    std::vector<int> rows_cols_per_proccess(num_proc);
    std::vector<int> displs(num_proc);

    
    // Find nr of rows/cols per proccess (same distribution both for rows and colls)
    // Use to find displacement into first row. 
    rows_cols_per_proccess[0] = global_dim / num_proc + (0 < global_dim % num_proc ? 1 : 0);
    displs[0] = 0;
    
    for (int i = 1; i < num_proc; ++i) {
        rows_cols_per_proccess[i] = global_dim / num_proc + (i < global_dim % num_proc ? 1 : 0);
        displs[i] = displs[i - 1] + rows_cols_per_proccess[i - 1];
    }


    // Create sendtypes per destination
    std::vector<MPI_Datatype> send_types(num_proc);
    for (int i = 0; i < num_proc; ++i) {
        MPI_Type_vector(                // Memory stencil for row send order
            elements->nrOfRows,         // count
            rows_cols_per_proccess[i],  // blocklength
            global_dim,                 // stride
            MPI_INT,                    // basetype
            &send_types[i]);
        MPI_Type_commit(&send_types[i]);
    }

    // Create receive types per source which shall transpose the received data to simplify sorting
    MPI_Datatype column_vector;
    MPI_Type_vector(                // Memory stencil for row send order
        elements->nrOfRows,         // count
        1,                          // blocklength
        global_dim,                 // stride
        MPI_INT,                    // basetype
        &column_vector);
    MPI_Type_commit(&column_vector);
    
    std::vector<MPI_Datatype> recv_types(num_proc);
    for (int i = 0; i < num_proc; ++i) {
        MPI_Type_create_hvector(        // Memory stencil for column receive order
            rows_cols_per_proccess[i],  // count
            1,                          // blocklength
            sizeof(MPI_INT),            // stride
            column_vector,              // basetype
            &recv_types[i]
        );

        MPI_Type_commit(&recv_types[i]);
    }

    // Setup remaining Alltoallw data
    std::vector<int> send_counts(num_proc);
    std::vector<int> send_displs(num_proc);
    std::vector<int> recv_counts(num_proc);
    std::vector<int> recv_displs(num_proc);
    matrix2D transpose_buffer(elements->nrOfRows, elements->nrOfCols);

    for (int i = 0; i < num_proc; ++i) {
        send_counts[i] = 1;                             // just one vector type
        send_displs[i] = displs[i] * sizeof(MPI_INT);   // byte-offset as we use MPI_Alltoallw
        recv_counts[i] = 1;                             // just one contigious type
        recv_displs[i] = displs[i] * sizeof(MPI_INT);                
    }

    int col_steps = ceil(log2(global_dim));
    int row_steps = col_steps + 1; 

    for (int i = 0; i < row_steps; i++){
        if (DDBUG){
            if (myid == ROOT){
                std::cout << myid << ": " << "Before row sort" << std::endl;
                elements->toString();
            }
        }
        for (int row = first_row_index; row < ((int)elements->nrOfRows + first_row_index); row++){
            if (row % 2 == 0){
                if (DDBUG){
                    // std::cout << myid << ": " << "Sorting even row" << std::endl;
                }
                std::sort(elements->data.begin() + global_dim * (row - first_row_index), elements->data.begin() + global_dim * (row - first_row_index + 1));
            }
            else {
                if (DDBUG){
                    // std::cout << myid << ": " << "Sorting odd row" << std::endl;
                }
                std::sort(elements->data.begin() + global_dim * (row - first_row_index), elements->data.begin() + global_dim * (row - first_row_index + 1), std::greater<int>());
            }
        }

        if (DDBUG){
            if (myid == ROOT){
                std::cout << myid << ": " << "Done with row sort" << std::endl; 
                elements->toString();
            }
        }

        // if (i < col_steps){
        //     // Transpose to treat columns as rows
        //     MPI_Alltoallw(
        //         MPI_IN_PLACE, send_counts.data(), send_displs.data(), send_types.data(),
        //         elements->data.data(), recv_counts.data(), recv_displs.data(), recv_types.data(),
        //         MPI_COMM_WORLD
        //     );

        //     if (DDBUG){
        //         if (myid == ROOT){
        //             std::cout << myid << ": " << "Before column sort" << std::endl; 
        //             elements->toString();
        //         }
        //     }
            
        //     for (int col = 0; col < (int)elements->nrOfRows; col++){
        //         std::sort(elements->data.begin() + global_dim * col,elements->data.begin() + global_dim * (col+1));
        //     }

        //     if (DDBUG){
        //         if (myid == ROOT){
        //             std::cout << myid << ": " << "Done with column sort" << std::endl; 
        //             elements->toString();
        //         }
        //     }

        //     // Transpose back to original layout
        //     MPI_Alltoallw(
        //         MPI_IN_PLACE, send_counts.data(), send_displs.data(), send_types.data(),
        //         elements->data.data(), recv_counts.data(), recv_displs.data(), recv_types.data(),
        //         MPI_COMM_WORLD
        //     );
        // }

        if (i < col_steps){
            // Transpose to treat columns as rows
            MPI_Alltoallw(
                elements->data.data(), send_counts.data(), send_displs.data(), send_types.data(),
                transpose_buffer.data.data(), recv_counts.data(), recv_displs.data(), recv_types.data(),
                MPI_COMM_WORLD
            );

            if (DDBUG){
                if (myid == ROOT){
                    std::cout << myid << ": " << "Before column sort" << std::endl; 
                    transpose_buffer.toString();
                }
            }
            
            for (int col = 0; col < (int)transpose_buffer.nrOfRows; col++){
                std::sort(transpose_buffer.data.begin() + global_dim * col, transpose_buffer.data.begin() + global_dim * (col+1));
            }

            if (DDBUG){
                if (myid == ROOT){
                    std::cout << myid << ": " << "Done with column sort" << std::endl; 
                    transpose_buffer.toString();
                }
            }

            // Transpose back to original layout
            MPI_Alltoallw(
                transpose_buffer.data.data(), send_counts.data(), send_displs.data(), send_types.data(),
                elements->data.data(), recv_counts.data(), recv_displs.data(), recv_types.data(),
                MPI_COMM_WORLD
            );
        }
    }

    // Cleanup resources
    for (int i = 0; i < num_proc; ++i) {
        MPI_Type_free (&send_types[i]);
        MPI_Type_free (&recv_types[i]);
    }
    MPI_Type_free (&column_vector);
    
    
}

void gather_on_root(matrix2D *all_elements, matrix2D *local_elements){
	int num_proc, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);


    // Init buffers used by ROOT 
    std::vector<int> recv_counts;
    std::vector<int> displs;

    if (myid == ROOT) {
        // Resize to avoid allocation of memory in all processes
        recv_counts.resize (num_proc);
        displs.resize(num_proc);

        // Shall be equal as long as matrix is square
        int nrOfRows = all_elements->nrOfRows;
        int nrOfCols = all_elements->nrOfCols;

        // Initialize first process info
        recv_counts[0] = (nrOfRows / num_proc + (0 < nrOfRows % num_proc ? 1 : 0)) * nrOfCols;
        displs[0] = 0;

        // initilize the rest based on previous entries
		for (int i = 1; i < num_proc; ++i) {
            recv_counts[i] = (nrOfRows / num_proc + (i < nrOfRows % num_proc ? 1 : 0)) * nrOfCols;
            displs[i] = displs[i - 1] + recv_counts[i - 1];
		}
	}

    // Gather all rows on root
	MPI_Gatherv(local_elements->data.data(), local_elements->n, MPI_INT,
                all_elements->data.data(), recv_counts.data(), displs.data(), MPI_INT,
				ROOT, MPI_COMM_WORLD
    );
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
        if (DDBUG){
            std::cout << "Warning: Output disabled" << std::endl;
        }
    }
	return 0;
}


int sorted_snake(matrix2D *elements){
    int dim = elements->nrOfCols;
    int prev = 0;
    
    if (DDBUG){
        std::cout << "Checking sorted " << std::endl; // Print each word
        elements->toString();
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