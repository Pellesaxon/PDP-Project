CC = mpic++
CFLAGS =  -Wall -O3 -ggdb
BINS = shearsort

all: $(BINS)

shearsort: shearSort.h shearSort.cpp
	$(CC) $(CFLAGS) -o $@ shearSort.cpp

clean:
	$(RM) $(BINS)
	$(RM) *.o
	$(RM) output_test.txt
	$(RM) testoutput*
	$(RM) core.*


run: shearsort
	mpirun -np 2 ./shearsort input/input8.txt testoutput8.txt

test: shearsort
	mpirun -np 2 ./shearsort input/input15.txt testoutput15.txt
	mpirun -np 2 ./shearsort input/input16.txt testoutput16.txt
	mpirun -np 2 ./shearsort 24 testoutput.txt


