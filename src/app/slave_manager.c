#include "slave_manager.h"

#include "utils.h"

#define ERROR_FREE_EXIT(adt, msg) \
	{                             \
		free_adt(adt);            \
		ERROR_EXIT(msg);          \
	}

struct slave_manager_cdt {
	char** filenames;
	int qfiles;
	int qslaves;
	int files_per_slave;
	int* slave_pids;
	int* fd_read;
	int* fd_write;

	int qfiles_sent;
	int av_slaves;
	int ret_files;

	int* active_files;
	char* has_data;
	/* data */
};

static void _send_file(SlaveManager adt, int idx);
static void _close_extra_pipes(int idx, SlaveManager adt);
static int _get_id_withdata(SlaveManager adt);

fd_set set;
int maxfd = 0;

void free_adt(SlaveManager adt) {
	if (adt == NULL)
		return;

	int write_flag = adt->fd_write != NULL;
	int read_flag = adt->fd_read != NULL;
	int slave_flag = adt->slave_pids != NULL;
	int data_flag = adt->has_data != NULL;
	int active_flag = adt->active_files != NULL;

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
	if (active_flag)
		free(adt->active_files);

	free(adt);
}

SlaveManager new_manager(char** filenames, int count, int qslaves, int files_per_slave) {
	if (count < 1 || filenames == NULL)
		ERROR_EXIT("new_manager");

	SlaveManager sm = calloc(1, sizeof(struct slave_manager_cdt));
	if (sm == NULL)
		ERROR_EXIT("calloc");

	sm->filenames = filenames;
	sm->qfiles = count;
	sm->qslaves = qslaves;
	sm->files_per_slave = files_per_slave;

	sm->has_data = calloc(qslaves, sizeof(char));
	if (sm->has_data == NULL)
		ERROR_FREE_EXIT(sm, "calloc");

	sm->slave_pids = malloc(sizeof(int) * qslaves);
	if (sm->slave_pids == NULL)
		ERROR_FREE_EXIT(sm, "malloc");

	sm->fd_read = malloc(sizeof(int) * qslaves);
	if (sm->fd_read == NULL)
		ERROR_FREE_EXIT(sm, "malloc");

	sm->fd_write = malloc(sizeof(int) * qslaves);
	if (sm->fd_write == NULL)
		ERROR_FREE_EXIT(sm, "malloc");

	sm->active_files = calloc(qslaves, sizeof(int));
	if (sm->active_files == NULL)
		ERROR_FREE_EXIT(sm, "calloc");

	return sm;
}

void init_slaves(SlaveManager adt) {
	if (adt == NULL)
		ERROR_EXIT("init_slaves");

	for (int i = 0; i < adt->qslaves; i++) {
		int sm[2];  // slave to master
		int ms[2];  // master to slave

		if (pipe(sm) == -1 || pipe(ms) == -1)
			ERROR_FREE_EXIT(adt, "pipe");

		int pid = fork();

		switch (pid) {
			case ERROR: {
				ERROR_FREE_EXIT(adt, "fork");
			} break;

			case CHILD: {
				if (close(ms[WRITE]) == -1 || close(sm[READ]) == -1)
					ERROR_FREE_EXIT(adt, "close");

				_close_extra_pipes(i, adt);

				dup2(ms[READ], STDIN_FILENO);
				close(ms[READ]);
				dup2(sm[WRITE], STDOUT_FILENO);
				close(sm[WRITE]);

				char* argv[] = {"./build/slave", NULL};
				execve("./build/slave", argv, NULL);

				ERROR_FREE_EXIT(adt, "execve");
			} break;

			default: {
				if (close(sm[WRITE]) == -1 || close(ms[READ]) == -1)
					ERROR_FREE_EXIT(adt, "close");

				adt->fd_read[i] = sm[READ];
				adt->fd_write[i] = ms[WRITE];
				adt->slave_pids[i] = pid;
				for (int j = 0; j < adt->files_per_slave; j++)
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
		ERROR_FREE_EXIT(adt, "ret_file");

	if (adt->av_slaves <= 0) {
		// si no hay data disponible -> toca usar select y quedar bloqueado hasta que termine algún slave
		FD_ZERO(&set);

		for (int i = 0; i < adt->qslaves; i++) {
			FD_SET(adt->fd_read[i], &set);
			maxfd = maxfd >= adt->fd_read[i] ? maxfd : adt->fd_read[i];
		}

		int q_fds = select(maxfd + 1, &set, NULL, NULL, NULL);
		if (q_fds == ERROR)
			ERROR_FREE_EXIT(adt, "select");

		adt->av_slaves = q_fds;

		for (int i = 0; i < adt->qslaves; i++) {
			if (FD_ISSET(adt->fd_read[i], &set))
				adt->has_data[i] = 1;
		}
	}

	int idx = _get_id_withdata(adt);
	if (idx == -1)
		return 0;

	// vamos a consumir
	adt->has_data[idx] = 0;
	adt->av_slaves -= 1;
	adt->ret_files += 1;

	int read_bytes = read(adt->fd_read[idx], buf, BUF_SIZE - 2);

	int i = 0;
	for (i = 0; i < read_bytes && buf[i] != '\n'; i++)
		continue;
	buf[i] = '\0';

	adt->active_files[idx] -= 1;

	if (adt->qfiles_sent < adt->qfiles && adt->active_files[idx] == 0)
		_send_file(adt, idx);

	return adt->slave_pids[idx];  // pid del esclavo
}

static void _send_file(SlaveManager adt, int idx) {
	if (adt == NULL || idx < 0)
		ERROR_FREE_EXIT(adt, "_send_file");
	if (adt->qfiles_sent >= adt->qfiles)
		return;

	int size = strlen(adt->filenames[adt->qfiles_sent]);
	if (write(adt->fd_write[idx], adt->filenames[adt->qfiles_sent], size) != size)
		fprintf(stderr, "Write not working");
	write(adt->fd_write[idx], "\n", 1);

	adt->qfiles_sent++;
	adt->active_files[idx] += 1;
}

static void _close_extra_pipes(int idx, SlaveManager adt) {
	for (int i = 0; i < idx; i++) {
		if (close(adt->fd_read[i]) == -1 || close(adt->fd_write[i]) == -1)
			ERROR_FREE_EXIT(adt, "close");
	}
}

static int _get_id_withdata(SlaveManager adt) {
	for (int i = 0; i < adt->qslaves; i++) {
		if (adt->has_data[i])
			return i;
	}
	return -1;
}
