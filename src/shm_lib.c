// #include "shm_lib.h"

// #include <stdio.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>
// #include <sys/types.h>

// #include "utils.h"

// // TODO: shmget -> shmopen

// static int _get_shared_block(char *filename, int size) {
// 	key_t key;

// 	// key numerico del filename
// 	key = ftok(filename, 0);

// 	if (key == ERROR) {
// 		error_exit("Error in ftok");
// 	}

// 	//-1 error o id del bloque creado
// 	return shmget(key, size, 0644 | IPC_CREAT);
// }

// char *attach_memory_block(char *filename, int size) {
// 	int shared_block_id = _get_shared_block(filename, size);
// 	char *result;

// 	if (shared_block_id == ERROR) {
// 		error_exit("Error in shmget");
// 	}

// 	result = shmat(shared_block_id, NULL, 0);

// 	if (result == (char *)ERROR) {
// 		error_exit("Error in shmat");
// 	}

// 	return result;
// }

// char detach_memory_block(char *block) {
// 	return (shmdt(block) != ERROR);
// }

// char destroy_memory_block(char *filename) {
// 	int shared_block_id = _get_shared_block(filename, 0);

// 	if (shared_block_id == ERROR) {
// 		error_exit("Error in shmget");
// 	}
// 	return (shmctl(shared_block_id, IPC_RMID, NULL) != ERROR);
// }

#include "shm_lib.h"

#include <fcntl.h>
#include <memory.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

#define BUF_SIZE  4096
#define PATH_SIZE 30

struct timespec ts;

struct shared_memory_cdt {
	sem_t sem;
	int fd;
	size_t widx, ridx;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
};

SharedMemory sm_create(char* path) {
	int fd = shm_open(path, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1)
		error_exit("shm_open");
	if (ftruncate(fd, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("ftruncate");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		error_exit("mmap");

	sm->widx = sm->ridx = 0;
	sm->fd = fd;
	strcpy(sm->path, path);

	if (sem_init(&sm->sem, 1, 0) == -1)
		error_exit("sem_init");

	return sm;
}

SharedMemory sm_join(char* path) {
	int fd = shm_open(path, O_RDWR, 0);
	if (fd == -1)
		error_exit("shm_open");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		error_exit("mmap");

	return sm;
}

void sm_write(SharedMemory self, char* buf, int size) {
	memcpy(self->buf + self->widx, buf, size + 1);
	self->widx += size + 1;
	if (sem_post(&self->sem) == -1)
		error_exit("sem_post");
}

int sm_read(SharedMemory self, char* buf) {
	// TODO: SACAR ESTO EL WAIT NO TIENE QUE SER TIMED
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 10;
	if (sem_timedwait(&self->sem, &ts) == -1)
		error_exit("sem_wait");
	int len = get_line(self->buf + self->ridx, buf);
	self->ridx += len + 1;
	return len;
}

void sm_destroy(SharedMemory self) {
	char path[PATH_SIZE];
	strcpy(path, self->path);
	int fd = self->fd;

	if (sem_destroy(&self->sem) == -1)
		error_exit("sem_close");
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("munmap");
	if (shm_unlink(path) == -1)
		error_exit("shm_unlink");
	if (close(fd) == -1)
		error_exit("close");
}

void sm_close(SharedMemory self) {
	int fd = self->fd;

	if (sem_destroy(&self->sem) == -1)
		error_exit("sem_close");
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("munmap");
	if (close(fd) == -1)
		error_exit("close");
}
