#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFSIZE 256

int main(int argc, char *argv[]) {
	FILE *f;
	char command[BUFFSIZE] = {0};
	char filepath[BUFFSIZE] = {0};
	char output[BUFFSIZE] = {0};

	while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
		sprintf(command, "md5sum %s", filepath);
		f = popen(command, "r");

		if (f == NULL) {
			perror("popen");
			exit(1);
		}

		while (fgets(output, sizeof(output), f) != NULL)
			dprintf(STDOUT_FILENO, "%s", output);

		pclose(f);
	}

	return 0;
}
