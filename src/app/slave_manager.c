
#include "slave_manager.h"

typedef struct slave_managerCDT {
	const char** filenames;

	int qfiles;
	int qslaves;
	int* slave_pids;
	int* fd_read;
	int* fd_write;

	int qfiles_sent;
	int av_files;
	int ret_files;
	int max_fd;
	int* has_data;
	/* data */
} slave_managerCDT;

void error_free_exit(slave_managerADT adt, char* msg);
void send_file(slave_managerADT adt, int idx);
void close_extra_pipes(int idx, slave_managerADT adt);
int get_id_withdata(slave_managerADT adt);

void error_exit(char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
void free_adt(slave_managerADT adt) {
	if (adt == NULL) {
		return;
	}
	int write_flag = adt->fd_write != NULL;
	int read_flag = adt->fd_read != NULL;
	int slave_flag = adt->slave_pids != NULL;

	int data_flag = adt->has_data != NULL;

	// free could be called if memory allocation failed

	for (int i = 0; i < adt->qslaves && write_flag && read_flag; i++) {
		close(adt->fd_write[i]);
		close(adt->fd_read[i]);
	}
	if (slave_flag)
		free(adt->slave_pids);
	if (data_flag)
		free(adt->has_data);
	if (write_flag)
		free(adt->fd_write);
	if (read_flag)
		free(adt->fd_read);

	free(adt);
}
slave_managerADT new_manager(const char** filenames, int count, int qslaves) {
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

	sm->av_files = 0;

	sm->ret_files = 0;
	sm->max_fd = 0;
	sm->qslaves = qslaves;

	sm->has_data = calloc(qslaves, sizeof(int));

	if (sm->has_data == NULL) {
		free(sm);
		error_exit("Memory allocation error [slave_pids]");
	}
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

void init_slaves(slave_managerADT adt) {
	if (adt == NULL) {
		error_exit("invalid parameters");
	}

	// pipes
	for (int i = 0; i < adt->qslaves; i++) {
		int sm[2];  // slave to master
		int ms[2];  // master to slave

		if (pipe(sm) == -1 || pipe(ms) == -1) {
			error_free_exit(adt, "Pipe Error");
		}

		int pid = fork();

		switch (pid) {
			case ERROR:
				/* code */
				error_free_exit(adt, "Fork Error");

				break;
			case CHILD:

				if (close(ms[WRITE]) == -1 || close(sm[READ]) == -1) {
					error_free_exit(adt, "Error Close Pipe");
				}

				close_extra_pipes(i, adt);

				dup2(ms[READ], STDIN_FILENO);
				close(ms[READ]);
				dup2(sm[WRITE], STDOUT_FILENO);
				close(sm[WRITE]);

				char* argv[] = {"./slave", NULL};
				execve("./build/slave", argv, NULL);

				error_free_exit(adt, "Error tratando de ejecutar ./slave");

				break;
			default:

				if (close(sm[WRITE]) == -1 || close(ms[READ]) == -1) {
					error_free_exit(adt, "Error Close Pipe");
				}

				adt->fd_read[i] = sm[READ];
				adt->fd_write[i] = ms[WRITE];
				if (adt->fd_read[i] > adt->max_fd)
					adt->max_fd = adt->fd_read[i];
				adt->slave_pids[i] = pid;
				printf("max_fd: %d | new fd_read[%d] = %d \n", adt->max_fd, i, adt->fd_read[i]);

				send_file(adt, i);

				break;
		}
	}
	printf("initialized slaves ! \n");
}

int has_next_file(slave_managerADT adt) {
	return adt->qfiles > adt->ret_files;
}

int ret_file(slave_managerADT adt, char buf[BUFFSIZE]) {
	if (buf == NULL || adt == NULL) {
		error_free_exit(adt, "Invalid Parameters");
	}

	if (adt->av_files <= 0) {
		fd_set set;

		// si no hay data disponible -> toca usar select y quedar bloqueado hasta que termine algún slave
		FD_ZERO(&set);

		for (int i = 0; i < adt->qslaves; i++) {
			FD_SET(adt->fd_read[i], &set);
		}

		int q_fds = select(adt->max_fd + 1, &set, NULL, NULL, NULL);

		if (q_fds == ERROR) {
			error_free_exit(adt, "ERROR in select()");
		}

		adt->av_files = q_fds;

		// update has_data
		for (int i = 0; i < adt->qslaves; i++) {
			if (FD_ISSET(adt->fd_read[i], &set)) {
				adt->has_data[i] = 1;
			}
		}
	}

	int idx = get_id_withdata(adt);

	// vamos a consumir
	adt->has_data[idx] = 0;
	adt->av_files -= 1;
	adt->ret_files += 1;

	int i = 0;
	char c;

	while ((read(adt->fd_read[idx], &c, 1) > 0) && c != '\n' && c != '\0' && i < (BUFFSIZE - 2)) {
		buf[i++] = c;
	}

	buf[i++] = '\n';
	buf[i++] = '\0';

	if (adt->qfiles_sent < adt->qfiles) {
		printf("sending new file \n");
		send_file(adt, idx);
	}

	return i;  // cuanto se escribió en el buffer
}
int get_id_withdata(slave_managerADT adt) {
	for (int i = 0; i < adt->qslaves; i++) {
		if (adt->has_data[i])
			return i;
	}
	return -1;
}
void send_file(slave_managerADT adt, int idx) {
	if (adt == NULL || idx < 0) {
		error_exit("Invalid parameters");
	}
	int size = strlen(adt->filenames[adt->qfiles_sent]);
	if (write(adt->fd_write[idx], adt->filenames[adt->qfiles_sent], size) != size) {
		printf("Write not working");
	}
	write(adt->fd_write[idx], "\n", 1);
	printf("file sent \n");
	(adt->qfiles_sent)++;
}

void close_extra_pipes(int idx, slave_managerADT adt) {
	for (int i = 0; i < idx; i++) {
		if (close(adt->fd_read[i]) == -1 || close(adt->fd_write[i]) == -1) {
			error_free_exit(adt, "Error Close Pipe");
		}
	}
}

void error_free_exit(slave_managerADT adt, char* msg) {
	free_adt(adt);
	error_exit(msg);
}