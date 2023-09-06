#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFSIZE 256

int main(int argc, char const *argv[]) {
	char *r;

	FILE *fptr;

	fptr = fopen("out.txt", "w");

	do {
		char buffer[BUFFSIZE] = {0};
		r = fgets(buffer, BUFFSIZE, stdin);
		int n = strlen(buffer);

		fprintf(fptr, "%s \n", buffer);

	} while (r != NULL);
	fclose(fptr);
	return 0;
}
