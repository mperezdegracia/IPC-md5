#include "slave_manager.h"

#include "utils.h"

struct slave_manager_cdt {
	char** filenames;
	int qfiles;
	int qslaves;
	int* slave_pids;
	int* fd_read;
	int* fd_write;

	int qfiles_sent;
	int av_files;
	int ret_files;
	int max_fd;

	char* has_data;
	/* data */
};

static void _error_free_exit(SlaveManager adt, char* msg);
static void _send_file(SlaveManager adt, int idx);
static void _close_extra_pipes(int idx, SlaveManager adt);
static int _get_id_withdata(SlaveManager adt);

void free_adt(SlaveManager adt) {
	if (adt == NULL)
		return;

	int write_flag = adt->fd_write != NULL;
	int read_flag = adt->fd_read != NULL;
	int slave_flag = adt->slave_pids != NULL;
	int data_flag = adt->has_data != NULL;

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

SlaveManager new_manager(char** filenames, int count, int qslaves) {
	if (count < 1 || filenames == NULL)
		error_exit("new_manager");

	SlaveManager sm = calloc(1, sizeof(struct slave_manager_cdt));
	if (sm == NULL)
		error_exit("calloc");

	sm->filenames = filenames;
	sm->qfiles = count;
	sm->qslaves = qslaves;

	sm->has_data = calloc(qslaves, sizeof(char));
	if (sm->has_data == NULL)
		_error_free_exit(sm, "malloc");

	sm->slave_pids = malloc(sizeof(int) * qslaves);
	if (sm->slave_pids == NULL)
		_error_free_exit(sm, "malloc");

	sm->fd_read = malloc(sizeof(int) * qslaves);
	if (sm->fd_read == NULL)
		_error_free_exit(sm, "malloc");

	sm->fd_write = malloc(sizeof(int) * qslaves);
	if (sm->fd_write == NULL)
		_error_free_exit(sm, "malloc");

	return sm;
}

void init_slaves(SlaveManager adt) {
	if (adt == NULL)
		error_exit("init_slaves");

	for (int i = 0; i < adt->qslaves; i++) {
		int sm[2];  // slave to master
		int ms[2];  // master to slave

		if (pipe(sm) == -1 || pipe(ms) == -1)
			_error_free_exit(adt, "pipe");

		int pid = fork();

		switch (pid) {
			case ERROR: {
				_error_free_exit(adt, "fork");
			} break;

			case CHILD: {
				if (close(ms[WRITE]) == -1 || close(sm[READ]) == -1)
					_error_free_exit(adt, "close");

				_close_extra_pipes(i, adt);

				dup2(ms[READ], STDIN_FILENO);
				close(ms[READ]);
				dup2(sm[WRITE], STDOUT_FILENO);
				close(sm[WRITE]);

				char* argv[] = {"./build/slave", NULL};
				execve("./build/slave", argv, NULL);

				_error_free_exit(adt, "execve");
			} break;

			default: {
				if (close(sm[WRITE]) == -1 || close(ms[READ]) == -1)
					_error_free_exit(adt, "close");

				adt->fd_read[i] = sm[READ];
				adt->fd_write[i] = ms[WRITE];
				if (adt->fd_read[i] > adt->max_fd)
					adt->max_fd = adt->fd_read[i];

				adt->slave_pids[i] = pid;
				_send_file(adt, i);
			} break;
		}
	}
}

int has_next_file(SlaveManager adt) {
	return adt->qfiles > adt->ret_files;
}

int ret_file(SlaveManager adt, char* buf) {
	if (buf == NULL || adt == NULL)
		_error_free_exit(adt, "ret_file");

	if (adt->av_files <= 0) {
		fd_set set;

		// si no hay data disponible -> toca usar select y quedar bloqueado hasta que termine algún slave
		FD_ZERO(&set);

		for (int i = 0; i < adt->qslaves; i++)
			FD_SET(adt->fd_read[i], &set);

		int q_fds = select(adt->max_fd + 1, &set, NULL, NULL, NULL);
		if (q_fds == ERROR)
			_error_free_exit(adt, "select");

		adt->av_files = q_fds;

		for (int i = 0; i < adt->qslaves; i++) {
			if (FD_ISSET(adt->fd_read[i], &set))
				adt->has_data[i] = 1;
		}
	}

	int idx = _get_id_withdata(adt);

	// vamos a consumir
	adt->has_data[idx] = 0;
	adt->av_files -= 1;
	adt->ret_files += 1;

	int i = 0;
	int read_bytes = read(adt->fd_read[idx], buf, BUF_SIZE - 2);

	for (i = 0; i < read_bytes && buf[i] != '\n'; i++)
		continue;
	buf[i] = '\0';

	if (adt->qfiles_sent < adt->qfiles)
		_send_file(adt, idx);

	// return i;  // cuanto se escribió en el buffer
	return adt->slave_pids[idx];  // pid del esclavo
}

static void _error_free_exit(SlaveManager adt, char* msg) {
	free_adt(adt);
	error_exit(msg);
}

static void _send_file(SlaveManager adt, int idx) {
	if (adt == NULL || idx < 0)
		_error_free_exit(adt, "_send_file");

	int size = strlen(adt->filenames[adt->qfiles_sent]);
	if (write(adt->fd_write[idx], adt->filenames[adt->qfiles_sent], size) != size)
		fprintf(stderr, "Write not working");

	write(adt->fd_write[idx], "\n", 1);
	adt->qfiles_sent++;
}

static void _close_extra_pipes(int idx, SlaveManager adt) {
	for (int i = 0; i < idx; i++) {
		if (close(adt->fd_read[i]) == -1 || close(adt->fd_write[i]) == -1)
			_error_free_exit(adt, "close");
	}
}

static int _get_id_withdata(SlaveManager adt) {
	for (int i = 0; i < adt->qslaves; i++) {
		if (adt->has_data[i])
			return i;
	}
	return -1;
}
