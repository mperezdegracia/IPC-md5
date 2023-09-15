#ifndef __SHM_LIB_H__
#define __SHM_LIB_H__

typedef struct shared_memory_cdt* SharedMemory;

SharedMemory sm_create(char* path);
SharedMemory sm_join(char* path);
void sm_write(SharedMemory self, char* buf, int size);
int sm_read(SharedMemory self, char* buf);
char sm_eof(SharedMemory self);
void sm_destroy(SharedMemory self);
void sm_close(SharedMemory self);

#endif
