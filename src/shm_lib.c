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

struct shared_memory_cdt {
	sem_t sem;
	char view, eof;
	int fd;
	size_t widx, ridx;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
};

SharedMemory sm_create(char* path) {
	int fd = shm_open(path, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
		ERROR_EXIT("shm_open");
	if (ftruncate(fd, sizeof(struct shared_memory_cdt)) == -1)
		ERROR_EXIT("ftruncate");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		ERROR_EXIT("mmap");

	sm->view = sm->eof = 0;
	sm->fd = fd;
	sm->widx = sm->ridx = 0;
	strcpy(sm->path, path);

	if (sem_init(&sm->sem, 1, 0) == -1)
		ERROR_EXIT("sem_init");

	return sm;
}

SharedMemory sm_join(char* path) {
	int fd = shm_open(path, O_RDWR, 0);
	if (fd == -1)
		ERROR_EXIT("shm_open");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		ERROR_EXIT("mmap");

	// no quiero que dos procesos vista se ejecuten en simultáneo
	if (sm->view != 0) {
		sm->view++;  // incremento porque sm_close decrementa
		sm_close(sm);
		return NULL;
	}
	sm->view++;
	return sm;
}

void sm_write(SharedMemory self, char* buf, int size) {
	memcpy(self->buf + self->widx, buf, size + 1);
	self->widx += size + 1;
	if (sem_post(&self->sem) == -1)
		ERROR_EXIT("sem_post");
}

int sm_read(SharedMemory self, char* buf) {
	if (sem_wait(&self->sem) == -1)
		ERROR_EXIT("sem_wait");
	int len = get_line(self->buf + self->ridx, buf);
	self->ridx += len + 1;
	return len;
}

char sm_eof(SharedMemory self) {
	// se termino de leer cuando el writer no escribe mas y se consumio todo lo existente
	return self->eof && self->widx == self->ridx;
}

void sm_destroy(SharedMemory self) {
	// el writer ya no seguirá escribiendo
	self->eof = 1;

	char path[PATH_SIZE];
	strcpy(path, self->path);
	int fd = self->fd;

	if (sem_destroy(&self->sem) == -1)
		ERROR_EXIT("sem_destroy");
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		ERROR_EXIT("munmap");
	if (shm_unlink(path) == -1)
		ERROR_EXIT("shm_unlink");
	if (close(fd) == -1)
		ERROR_EXIT("close");
}

void sm_close(SharedMemory self) {
	int fd = self->fd;

	if (sem_destroy(&self->sem) == -1)
		ERROR_EXIT("sem_destroy");

	self->view--;
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		ERROR_EXIT("munmap");
	if (close(fd) == -1)
		ERROR_EXIT("close");
}
