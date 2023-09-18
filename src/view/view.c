#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shm_lib.h"
#include "utils.h"

#define BUF_SIZE 256

int main(int argc, char* argv[]) {
	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdout");

	char filename[BUF_SIZE];
	if (argc == 1) {
		char format[10];
		sprintf(format, "%%%ds", BUF_SIZE - 1);
		scanf(format, filename);
	} else if (argc == 2) {
		strncpy(filename, argv[1], BUF_SIZE - 1);
	} else {
		fprintf(stderr, "Called view with wrong parameters");
		exit(1);
	}

	SharedMemory sm = sm_join(filename);
	char buf[BUF_SIZE];

	while (sm_read(sm, buf) != 0)
		puts(buf);

	sm_close(sm);
	return 0;
}
