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
	if (argc <= 1) {
		fprintf(stderr, "Uso: %s archivo1 archivo2 ...\n", argv[0]);
	}

	printf("starting \n");

	int files_qty = argc - 1;

	const char** files = (const char**)&argv[1];

	slave_managerADT sm = new_manager(files, files_qty, SLAVES_QTY);

	init_slaves(sm);

	sleep(3);

	while (has_next_file(sm)) {
		char buf[BUFFSIZE] = {0};

		ret_file(sm, buf);
		printf("RESULT: %s \n", buf);
		sleep(1);
	}
	printf("\n FINISHED \n");
	free_adt(sm);

	return 0;
}
