#ifndef __SLAVE_MANAGER_H__
#define __SLAVE_MANAGER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD    0
#define READ     0
#define WRITE    1
#define BUF_SIZE 256

typedef struct slave_manager_cdt* SlaveManager;

SlaveManager new_manager(char** filenames, int count, int qslaves, int files_per_slave);
void init_slaves(SlaveManager adt);
void free_adt(SlaveManager adt);
int has_next_file(SlaveManager adt);
int ret_file(SlaveManager adt, char* buf);

#endif
