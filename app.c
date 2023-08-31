#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SLAVES_QTY 1

#define READ 0
#define WRITE 1

#define MS 0  // Master to Slave
#define SM 1  // Slave to Master

#define MAXPATH 256

void close_master_pipes(int id, int pipes[SLAVES_QTY][2][2]);

int create_slave(int id, int pipes[SLAVES_QTY][2][2]);

void create_pipes(int pipes[SLAVES_QTY][2][2]);

void close_other_slaves(int me, int fd[SLAVES_QTY][2][2]);

int
main(int argc, char const* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Uso: %s archivo1 archivo2 ...\n", argv[0]);
	}

	int files_qty = argc - 1;

	const char** files = &argv[1];

	//        slave_id |flow direction|R/W
	int pipes[SLAVES_QTY][2][2];

	create_pipes(pipes);

	int slave_pids[SLAVES_QTY] = { 0 };

	for (int i = 0; i < SLAVES_QTY; i++) {
		slave_pids[i] = create_slave(i, pipes);
		close_master_pipes(i, pipes);
	}

	// Distribuir archivos iniciales entre los esclavos

	int active_slaves = SLAVES_QTY;
	int file_idx = 1;

	for (int i = 0; i < SLAVES_QTY; i++) {
		if (file_idx < argc) {
			// todavía tengo archivos para mandar
			int size = strlen(argv[file_idx]) + 1;
			printf("[size: %d] :%s \n",size, argv[file_idx]);
			
			if (write(pipes[i][MS][WRITE], argv[file_idx], size) != size ) {
				printf("Write not working");

			}
			file_idx++;

		} else {
			// ya no tengo más archivos
			close(pipes[i][MS][WRITE]);
			pipes[i][MS][WRITE] = -1;
			active_slaves--;
		}
	}



	
	return 0;
}

void
close_master_pipes(int id, int pipes[SLAVES_QTY][2][2]) {
	close(pipes[id][MS][READ]);
	close(pipes[id][SM][WRITE]);
}
int
create_slave(int id, int pipes[SLAVES_QTY][2][2]) {
	pid_t pid = fork();

	char* argv[] = { "./slave", NULL };
	if (pid == -1) {
		perror("Error al crear proceso esclavo");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		// Código del proceso esclavo

		close_other_slaves(id, pipes);  // cierra todos los pipes de los slaves creados previamente
		close(pipes[id][MS][WRITE]);    // cerramos los sentidos que no nos sirven
		close(pipes[id][SM][READ]);

		dup2(pipes[id][MS][READ], STDIN_FILENO);
		dup2(pipes[id][SM][WRITE], STDOUT_FILENO);
		execve("./slave", argv, NULL);

		perror("error Creating slave");
		exit(EXIT_FAILURE);
	}

	return pid;
}
void
create_pipes(int pipes[SLAVES_QTY][2][2]) {
	for (int i = 0; i < SLAVES_QTY; i++) {
		for (int j = 0; j < 2; j++) {
			if ((pipe(pipes[i][j])) == -1) {
				perror("error al crear pipes");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void
close_other_slaves(int me, int fd[SLAVES_QTY][2][2]) {
	for (int i = 0; i < me; i++) {
		close(fd[i][SM][READ]);
		close(fd[i][SM][WRITE]);
		close(fd[i][MS][READ]);
		close(fd[i][MS][WRITE]);
	}
}