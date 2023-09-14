#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shm_lib.h"
#include "slave_manager.h"
#include "utils.h"

#define SLAVES_QTY  2
#define OUTPUT_FILE "out_app.txt"

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: %s [FILES...]\n", argv[0]);
		exit(1);
	}

	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdout");

	// puts("starting");
	shm_unlink("/shm_tpe_so");
	SlaveManager sm = new_manager(argv + 1, argc - 1, SLAVES_QTY);
	init_slaves(sm);
	SharedMemory shm = sm_create("/shm_tpe_so");
	puts("/shm_tpe_so");
	sleep(3);

	char buf[BUFFSIZE] = {0};

	while (has_next_file(sm)) {
		ret_file(sm, buf);
		printf("RESULT: %s\n", buf);
		sleep(1);
		sm_write(shm, buf, strlen(buf));
	}

	free_adt(sm);
	// sleep(2);
	puts("finished");
	sm_destroy(shm);
	return 0;
}
