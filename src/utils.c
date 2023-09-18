#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int get_line(char *src, char *tgt) {
	int i;
	for (i = 0; src[i] != '\n' && src[i] != '\0'; i++)
		tgt[i] = src[i];
	tgt[i] = '\0';
	return i;
}
