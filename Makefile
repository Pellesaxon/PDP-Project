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


run: shearsort
	mpirun -np 1 ./shearsort input/input16.txt testoutput16.txt

test: shearsort
	mpirun -np 1 ./shearsort input/input15.txt testoutput15.txt
	mpirun -np 1 ./shearsort input/input16.txt testoutput16.txt


