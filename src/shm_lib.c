#include "shm_lib.h"

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "utils.h"

// TODO: shmget -> shmopen

static int _get_shared_block(char *filename, int size) {
	key_t key;

	// key numerico del filename
	key = ftok(filename, 0);

	if (key == ERROR) {
		error_exit("Error in ftok");
	}

	//-1 error o id del bloque creado
	return shmget(key, size, 0644 | IPC_CREAT);
}

char *attach_memory_block(char *filename, int size) {
	int shared_block_id = _get_shared_block(filename, size);
	char *result;

	if (shared_block_id == ERROR) {
		error_exit("Error in shmget");
	}

	result = shmat(shared_block_id, NULL, 0);

	if (result == (char *)ERROR) {
		error_exit("Error in shmat");
	}

	return result;
}

char detach_memory_block(char *block) {
	return (shmdt(block) != ERROR);
}

char destroy_memory_block(char *filename) {
	int shared_block_id = _get_shared_block(filename, 0);

	if (shared_block_id == ERROR) {
		error_exit("Error in shmget");
	}
	return (shmctl(shared_block_id, IPC_RMID, NULL) != ERROR);
}