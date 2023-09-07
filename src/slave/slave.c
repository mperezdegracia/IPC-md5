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

int main(int argc, char const *argv[]) {
	int read_data;
	char buf[BUFFSIZE] = {0};
	do {
		
		read_data = fscanf(stdin, "%s", buf);
		sleep(1);
		//printf("%s\n", buf);
		dprintf(STDOUT_FILENO, "%s\n", buf);
		fflush(stdin);
		/* code */
	} while (read_data != EOF);

		// fprintf(stderr, "%s\n", filename);
		// dprintf(STDOUT_FILENO, "%s\n", filename);

	return 0;
}