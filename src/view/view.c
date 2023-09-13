#include <stdio.h>
#include <string.h>

#include "shm_lib.h"
#include "traffic_lights.h"
#include "utils.h"

#define MAX_AUTISM 1024

int main(int argc, char** argv) {
	char filename[MAX_AUTISM];

	if (argc == 1) {
		scanf("%s", filename);
	} else if (argc == 2) {
		strcpy(filename, argv[1]);
	} else {
		error_exit("Called view with wrong parameters");
	}
	puts(filename);
	//--------------------------
	char* blok = attach_memory_block(filename, BLOCK_SIZE);

	if (blok == NULL) {
		error_exit("Error in attach shm");
	}

	char sharma[MAX_AUTISM];

	sscanf(blok, "%s", sharma);

	puts(sharma);

	detach_memory_block(blok);
	//--------------------------
	return 0;
}
