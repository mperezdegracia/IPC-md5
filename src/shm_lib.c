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
	sem_t wsem, rsem;
	char view, eof;
	int fd;
	size_t widx, ridx;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
};

SharedMemory sm_create(char* path) {
	shm_unlink(path);

	int fd = shm_open(path, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1)
		error_exit("shm_open");
	if (ftruncate(fd, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("ftruncate");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		error_exit("mmap");

	sm->view = sm->eof = 0;
	sm->fd = fd;
	sm->widx = sm->ridx = 0;
	strcpy(sm->path, path);

	if (sem_init(&sm->wsem, 1, 0) == -1)
		error_exit("sem_init-wsem");
	if (sem_init(&sm->rsem, 1, 0) == -1)
		error_exit("sem_init-rsem");

	return sm;
}

SharedMemory sm_join(char* path) {
	int fd = shm_open(path, O_RDWR, 0);
	if (fd == -1)
		error_exit("shm_open");

	SharedMemory sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sm == MAP_FAILED)
		error_exit("mmap");

	// no quiero que dos procesos vista se ejecuten en simultáneo
	if (sm->view != 0) {
		sm_close(sm);
		return NULL;
	}
	sm->view++;

	return sm;
}

void sm_write(SharedMemory self, char* buf, int size) {
	memcpy(self->buf + self->widx, buf, size + 1);
	self->widx += size + 1;
	if (sem_post(&self->wsem) == -1)
		error_exit("sem_post-wsem");
}

int sm_read(SharedMemory self, char* buf) {
	if (self->eof)
		return 0;
	if (sem_wait(&self->wsem) == -1)
		error_exit("sem_wait-wsem");
	int len = get_line(self->buf + self->ridx, buf);
	self->ridx += len + 1;
	return len;
}

char sm_eof(SharedMemory self) {
	return self->eof;
}

void sm_destroy(SharedMemory self) {
	self->eof = 1;
	if (sem_wait(&self->rsem) == -1)
		error_exit("sem_wait");

	char path[PATH_SIZE];
	strcpy(path, self->path);
	int fd = self->fd;

	if (sem_destroy(&self->wsem) == -1)
		error_exit("sem_close-wsem");
	if (sem_destroy(&self->rsem) == -1)
		error_exit("sem_close-rsem");
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("munmap");
	if (shm_unlink(path) == -1)
		error_exit("shm_unlink");
	if (close(fd) == -1)
		error_exit("close");
}

void sm_close(SharedMemory self) {
	int fd = self->fd;

	if (sem_post(&self->rsem) == -1)
		error_exit("sem_post");

	if (sem_destroy(&self->wsem) == -1)
		error_exit("sem_close-wsem");
	if (sem_destroy(&self->rsem) == -1)
		error_exit("sem_close-rsem");
	if (munmap(self, sizeof(struct shared_memory_cdt)) == -1)
		error_exit("munmap");
	if (close(fd) == -1)
		error_exit("close");
}
