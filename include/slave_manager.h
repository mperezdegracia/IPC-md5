#ifndef __SLAVE_MANAGER_ADT_H__
#define __SLAVE_MANAGER_ADT_H__

/* Standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD 0
#define READ 0
#define WRITE 1
#define ERROR -1
#define MAXPATH 256
#define BUFFSIZE 256 

typedef struct slave_managerCDT* slave_managerADT;

slave_managerADT new_manager(const char** filenames, int count, int qslaves);
void init_slaves(slave_managerADT adt);
void free_adt(slave_managerADT adt);
int has_next_file(slave_managerADT adt);
int ret_file(slave_managerADT adt, char buf[BUFFSIZE]);
#endif