#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define BUF_SIZE 256

void md5(char *filename, char *output, int size);

int main(int argc, char *argv[]) {
	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		ERROR_EXIT("setvbuf/stdout");

	char filepath[BUF_SIZE];
	char output[BUF_SIZE];

	while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
		md5(filepath, output, sizeof(output));
		write(STDOUT_FILENO, output, BUF_SIZE - 2);
	}

	return 0;
}

void md5(char *filename, char *output, int size) {
	FILE *p;
	char command[BUF_SIZE * 2];

	snprintf(command, sizeof(command), "md5sum %s", filename);
	p = popen(command, "r");
	if (p == NULL)
		ERROR_EXIT("popen");

	if (fgets(output, size, p) == NULL) {
		pclose(p);
		ERROR_EXIT("fgets");
	}
	pclose(p);
}
