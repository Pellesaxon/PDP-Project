CC = mpic++
CFLAGS =  -Wall -O3 -ggdb
BINS = shearsort

all: $(BINS)

shearsort: shearsort.h shearsort.cpp
	-ml gcc openmpi
	$(CC) $(CFLAGS) -o $@ shearsort.cpp

clean:
	$(RM) $(BINS)
	$(RM) *.o
	$(RM) output_test.txt
	$(RM) testoutput.txt

test: shearsort
	mpirun -np 1 ./shearsort input/input15.txt testoutput.txt

# test: quicksort
# 	mpirun -np 1 ./quicksort input10.txt testoutput.txt 1
# 	mpirun -np 2 ./quicksort input10.txt testoutput.txt 1
# 	mpirun -np 2 ./quicksort input10.txt testoutput.txt 2
# 	mpirun -np 2 ./quicksort input10.txt testoutput.txt 3
# 	mpirun -np 4 ./quicksort input10test.txt testoutput.txt 1
# 	mpirun -np 4 ./quicksort input10test.txt testoutput.txt 2
# 	mpirun -np 4 ./quicksort input10test.txt testoutput.txt 3

