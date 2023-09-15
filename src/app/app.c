#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shm_lib.h"
#include "slave_manager.h"
#include "utils.h"

#define SLAVES_QTY  5
#define OUTPUT_FILE "tpe_so_output.txt"

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: %s [FILES...]\n", argv[0]);
		exit(1);
	}

	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdout");

	SharedMemory shm = sm_create("/shm_tpe_so");
	puts("/shm_tpe_so");

	SlaveManager sm = new_manager(argv + 1, argc - 1, SLAVES_QTY);
	init_slaves(sm);

	FILE *out = fopen(OUTPUT_FILE, "w");

	// espero para que se conecte la view
	sleep(2);

	char buf[BUF_SIZE] = {0};
	char send[BUF_SIZE + 10];
	int pid, len;

	while (has_next_file(sm)) {
		pid = ret_file(sm, buf);
		len = sprintf(send, "%i  %s", pid, buf);
		sm_write(shm, send, len);
		fprintf(out, "%s\n", send);
	}

	sm_destroy(shm);
	free_adt(sm);
	fclose(out);

	return 0;
}
