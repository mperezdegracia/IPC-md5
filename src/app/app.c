#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "slave_manager.h"

#define SLAVES_QTY  2
#define OUTPUT_FILE "out_app.txt"

int main(int argc, char* argv[]) {
	if (argc <= 1)
		fprintf(stderr, "Usage: %s [FILES...]\n", argv[0]);

	puts("starting");
	SlaveManager sm = new_manager(argv + 1, argc - 1, SLAVES_QTY);
	init_slaves(sm);
	sleep(3);

	while (has_next_file(sm)) {
		char buf[BUFFSIZE] = {0};
		ret_file(sm, buf);
		printf("RESULT: %s\n", buf);
		sleep(1);
	}

	free_adt(sm);
	puts("finished");

	return 0;
}
