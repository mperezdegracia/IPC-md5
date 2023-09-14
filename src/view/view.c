#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shm_lib.h"
#include "utils.h"

#define BUF_SIZE 256

int main(int argc, char* argv[]) {
	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdout");

	char filename[BUF_SIZE];
	if (argc == 1) {
		scanf("%s", filename);
	} else if (argc == 2) {
		strcpy(filename, argv[1]);
	} else {
		fprintf(stderr, "Called view with wrong parameters");
		exit(1);
	}

	// sleep(4);
	SharedMemory sm = sm_join(filename);
	char buf[BUF_SIZE];

	while (sm_read(sm, buf) != 0) {
		puts(buf);
		// sleep(2);
	}

	sm_close(sm);

	return 0;
}
