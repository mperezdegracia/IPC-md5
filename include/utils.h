#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>

#define ERROR -1

#define ERROR_EXIT(msg) \
	{                   \
		perror(msg);    \
		exit(1);        \
	}

int get_line(char *src, char *tgt);

#endif
