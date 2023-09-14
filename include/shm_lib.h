#ifndef __SHM_LIB_H__
#define __SHM_LIB_H__

/* Shared Memory */

// #define BLOCK_SIZE 4096
// #define FILENAME   "Makefile"

// char *attach_memory_block(char *filename, int size);
// char detach_memory_block(char *block);
// char destroy_memory_block(char *filename);

typedef struct shared_memory_cdt* SharedMemory;

SharedMemory sm_create(char* path);
SharedMemory sm_join(char* path);
void sm_write(SharedMemory self, char* buf, int size);
int sm_read(SharedMemory self, char* buf);
void sm_destroy(SharedMemory self);
void sm_close(SharedMemory self);

#endif
