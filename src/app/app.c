#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shm_lib.h"
#include "slave_manager.h"
#include "utils.h"

#define SLAVES_QTY      5
#define FILES_PER_SLAVE 5
#define OUTPUT_FILE     "tpe_so_output.txt"
#define SHM_PATH        "/shm_tpe_so"

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: %s [FILES...]\n", argv[0]);
		exit(1);
	}

	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdout");

	SharedMemory shm = sm_create(SHM_PATH);
	puts(SHM_PATH);

	SlaveManager sm = new_manager(argv + 1, argc - 1, SLAVES_QTY, FILES_PER_SLAVE);
	init_slaves(sm);

	FILE *out = fopen(OUTPUT_FILE, "w");
	if (out == NULL)
		ERROR_EXIT("fopen");

	// espero para que se conecte la view
	sleep(2);

	int len;

	while (has_next_file(sm)) {
		char buf[BUF_SIZE] = {0};
		len = ret_file(sm, buf);
		sm_write(shm, buf, len);
		fprintf(out, "%s\n", buf);
	}

	// send EOF and destroy
	sm_write(shm, "", strlen(""));
	sm_destroy(shm);
	free_adt(sm);
	fclose(out);

	return 0;
}
