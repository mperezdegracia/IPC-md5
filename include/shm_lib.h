#ifndef __SHM_LIB_H__
#define __SHM_LIB_H__

/* Shared Memory */

#define BLOCK_SIZE 4096
#define FILENAME   "/dev/shm/blok"

char *attach_memory_block(char *filename, int size);
char detach_memory_block(char *block);
char destroy_memory_block(char *filename);

#endif