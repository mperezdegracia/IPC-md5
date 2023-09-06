
#include "slave_manager.h"

typedef struct slave_managerCDT {
	const char** filenames;
	int qfiles_sent;
	int qfiles;
	int qslaves;
	int* slave_pids;
	int* fd_read;
	int* fd_write;

	/* data */
} slave_managerCDT;

void error_free_exit(slave_managerADT adt, char* msg);
void send_file(slave_managerADT adt, int idx);
void close_extra_pipes(int idx, slave_managerADT adt);


void
error_exit(char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
void
free_adt(slave_managerADT adt) {
	if (adt == NULL) {
		error_exit("invalid free");
	}
	int write_flag = adt->fd_write != NULL;
	int read_flag = adt->fd_read != NULL;

	// free could be called if memory allocation failed

	for (int i = 0; i < adt->qslaves && !write_flag && !read_flag; i++) {
		close(adt->fd_write[i]);
		close(adt->fd_read[i]);
	}

	if (write_flag)
		free(adt->fd_write);
	if (read_flag)
		free(adt->fd_read);

	if (adt->slave_pids != NULL)
		free(adt->slave_pids);

	free(adt);
}
slave_managerADT
new_manager(const char** filenames, int count, int qslaves) {
	if (count < 1 || filenames == NULL) {
		error_exit("invalid parameters");
	}

	slave_managerADT sm = calloc(1, sizeof(slave_managerCDT));

	if (sm == NULL) {
		error_exit("Memory allocation error [slave_manager]");
	}

	sm->filenames = filenames;

	sm->qfiles = count;

	sm->qfiles_sent = 0;

	sm->qslaves = qslaves;

	sm->slave_pids = malloc(sizeof(int) * qslaves);

	if (sm->slave_pids == NULL) {
		free(sm);
		error_exit("Memory allocation error [slave_pids]");
	}

	sm->fd_read = malloc(sizeof(int) * qslaves);

	if (sm->fd_read == NULL) {
		free_adt(sm);
		error_exit("Memory allocation error [slave_pids]");
	}
	sm->fd_write = malloc(sizeof(int) * qslaves);

	if (sm->fd_write == NULL) {
		free_adt(sm);
		error_exit("Memory allocation error [slave_pids]");
	}

	return sm;
}

void
init_slaves(slave_managerADT adt) {
	if (adt == NULL) {
		error_exit("invalid parameters");
	}

	// pipes
	for (int i = 0; i < adt->qslaves; i++) {
		int sm[2];  // slave to master
		int ms[2];  // master to slave

		if (pipe(sm) == -1 || pipe(ms) == -1) {
			free_adt(adt);
			error_exit("Pipe Error");
		}

		int pid = fork();

		switch (pid) {
			case ERROR:
				/* code */
				free_adt(adt);
				error_exit("Fork Error");
				break;
			case CHILD:

				if (close(ms[WRITE]) == -1 || close(sm[READ]) == -1) {
					error_free_exit(adt, "Error Close Pipe");
				}

				close_extra_pipes(i, adt);

				dup2(ms[READ], STDIN_FILENO);
				dup2(sm[WRITE], STDOUT_FILENO);

				char* argv[] = { "./slave", NULL };
				execve("./build/slave", argv, NULL);

				break;
			default:

				if (close(sm[WRITE]) == -1 || close(ms[READ]) == -1) {
					error_free_exit(adt, "Error Close Pipe");
				}

				adt->fd_read[i] = sm[READ];
				adt->fd_write[i] = ms[WRITE];
				adt->slave_pids[i] = pid;

				send_file(adt, i);
				send_file(adt, i);
				send_file(adt, i);
				send_file(adt, i);
				send_file(adt, i);
				break;
		}
	}
}

void
send_file(slave_managerADT adt, int idx) {
	if (adt == NULL || idx < 0) {
		error_exit("Invalid parameters");
	}
	int size = strlen(adt->filenames[adt->qfiles_sent]);
	if (write(adt->fd_write[idx], adt->filenames[adt->qfiles_sent], size) != size) {
		printf("Write not working");
	}
	write(adt->fd_write[idx], "\n", 1);
	adt->qfiles_sent++;
}

void
close_extra_pipes(int idx, slave_managerADT adt) {
	for (int i = 0; i < idx; i++) {
		if (close(adt->fd_read[i]) == -1 || close(adt->fd_write[i]) == -1) {
			error_free_exit(adt, "Error Close Pipe");
		}
	}
}

void
error_free_exit(slave_managerADT adt, char* msg) {
	free_adt(adt);
	error_exit(msg);
}
