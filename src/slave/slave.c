#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
	if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdin");
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		error_exit("setvbuf/stdout");

	FILE *p;

	char *md5 = "md5sum ";
	int md5len = strlen(md5);

	char filepath[BUF_SIZE];
	char command[BUF_SIZE + md5len];
	char output[BUF_SIZE];

	while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
		sprintf(command, "%s%s", md5, filepath);
		p = popen(command, "r");
		if (p == NULL)
			error_exit("popen");

		while (fgets(output, sizeof(output), p) != NULL)
			printf("%s", output);

		pclose(p);
	}

	return 0;
}
