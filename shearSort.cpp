#include "shearSort.h"


int main(int argc, char **argv) {
    int num_proc, myid, global_id, num_values;

    if (3 != argc) {
		printf("Usage: input_file output_file \n Amount of arguments submitted: %d\n", argc);
		return 1;
	}

    char *input_name = argv[1];
	char *output_name = argv[2];

    

}
