#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void error_exit(char* msg) {
	perror(msg);
	exit(ERROR);
}
