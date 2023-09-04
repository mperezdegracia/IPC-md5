#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "slave_manager.h"

#define SLAVES_QTY 1

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Uso: %s archivo1 archivo2 ...\n", argv[0]);
	}

	int files_qty = argc - 1;

	const char** files = &argv[1];

	slave_managerADT sm = new_manager(files, files_qty, SLAVES_QTY);

	init_slaves(sm);

	free_adt(sm);

	return 0;
}
