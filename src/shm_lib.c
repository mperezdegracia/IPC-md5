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
	char view;
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

	SharedMemory ret;
	ret.sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ret.sm == MAP_FAILED)
		ERROR_EXIT("mmap");

	ret.sm->view = 0;
	ret.fd = fd;
	ret.sm->widx = ret.sm->ridx = 0;
	strcpy(ret.sm->path, path);

	if (sem_init(&ret.sm->sem, 1, 0) == -1)
		ERROR_EXIT("sem_init");

	return ret;
}

SharedMemory sm_join(char* path) {
	int fd = shm_open(path, O_RDWR, 0);
	if (fd == -1)
		ERROR_EXIT("shm_open");

	SharedMemory ret;
	ret.sm = mmap(NULL, sizeof(struct shared_memory_cdt), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ret.sm == MAP_FAILED)
		ERROR_EXIT("mmap");

	ret.fd = fd;

	// no quiero que dos procesos vista se ejecuten en simultáneo
	if (ret.sm->view != 0) {
		ret.sm->view++;  // incremento porque sm_close decrementa
		sm_close(ret);
		fprintf(stderr, "Only one view should run at the same time\n");
		exit(1);
	}
	ret.sm->view++;
	return ret;
}

void sm_write(SharedMemory self, char* buf, int size) {
	memcpy(self.sm->buf + self.sm->widx, buf, size + 1);
	self.sm->widx += size + 1;
	if (sem_post(&self.sm->sem) == -1)
		ERROR_EXIT("sem_post");
}

int sm_read(SharedMemory self, char* buf) {
	if (sem_wait(&self.sm->sem) == -1)
		ERROR_EXIT("sem_wait");
	int len = get_line(self.sm->buf + self.sm->ridx, buf);
	self.sm->ridx += len + 1;
	return len;
}

void sm_destroy(SharedMemory self) {
	char path[PATH_SIZE];
	strcpy(path, self.sm->path);

	if (sem_destroy(&self.sm->sem) == -1)
		ERROR_EXIT("sem_destroy");
	if (munmap(self.sm, sizeof(struct shared_memory_cdt)) == -1)
		ERROR_EXIT("munmap");
	if (shm_unlink(path) == -1)
		ERROR_EXIT("shm_unlink");
	if (close(self.fd) == -1)
		ERROR_EXIT("close");
}

void sm_close(SharedMemory self) {
	if (sem_destroy(&self.sm->sem) == -1)
		ERROR_EXIT("sem_destroy");

	self.sm->view--;
	if (munmap(self.sm, sizeof(struct shared_memory_cdt)) == -1)
		ERROR_EXIT("munmap");
	if (close(self.fd) == -1)
		ERROR_EXIT("close");
}
