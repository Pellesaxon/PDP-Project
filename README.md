# PDP-Project: Shear Sort

Project files for individual project in Parallel and Distributed Programming VT25. Implementing the shear sort algorithm using MPI and more specifically using `MPI_Alltoallw()` in combination with MPI derived datatypes.

## Structure

`shearSort.cpp` Main project code. Implements main and methods described in header file.
`shearSort.h` Header file for the main project. Documents helper functions and defines helper class `matrix2D`.
`Makefile` Sores make-commands for building and running the project.
`inputs/` Stores some basic input files that can be used for validation. 
`makeMatrix.py` Script used to generate random inputs. 
`*.sh` Slurm job-scripts used to benchmark performance of project. 
`shearSort_reference.cpp` First working version of the sequential implementation. Planed to use it for scalability testing but ultimately just ran the parallel version with one process. 


## Usage

### Build the project
``` bash
make all
```

### Run a small scenario
Runs the algorithm with a 8x8 matrix input and 2 processes
``` bash
make run
```

### Run custom scenario:
``` bash
mpirun -np  [num processes] ./shearsort input=[order of matrix|input file] output=[output file]
```
Input files shall store the order of the square matrix as first item and then the matrix store in row-major order with space-separation.

### Toggle writing output:
Toggles if resulting sorted matrix shall be written to the output file. 
``` cpp
shearSort.h/ #define OUTPUT [0|1]
```

### Toggle debugging
Toggles if debug prints shall be written. 
``` cpp
shearSort.h/ #define DDBUG [0|1]
```

## Testing
Runs three small scenarios which test slightly different parts of the project. 
``` bash
$ make test
```

## Credits

- [Oskar Perers - author][link-author]
- Peer reviewed by [Adam Ross](https://github.com/r055a)

[link-author]: https://github.com/Pellesaxon/
