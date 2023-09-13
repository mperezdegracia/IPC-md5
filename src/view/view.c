#include <stdio.h>
#include <string.h>

#include "traffic_lights.h"
#include "utils.h"

#define MAX_AUTISM 1024

int main(int argc, char **argv) {
	char filename[MAX_AUTISM];

	if (argc == 1) {
		scanf("%s", filename);
	} else if (argc == 2) {
		strcpy(filename, argv[1]);
	} else {
		error_exit("Called view with wrong parameters");
	}
	puts(filename);
	return 0;
}
