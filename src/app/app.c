#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SLAVES_NUM 1

void prepare_slave(int id, int pipes[SLAVES_NUM][2]);

void create_pipes(int pipes[SLAVES_NUM][2]);

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Uso: %s archivo1 archivo2 ...\n", argv[0]);
	}

	int FILES_NUM = argc - 1;
	char** FILE_PATHS = &argv[1];

	//        slave_id |flow direction| R/W
	int pipes[SLAVES_NUM][2];

	create_pipes(pipes);

	for (int i = 0; i < SLAVES_NUM; i++) {
		prepare_slave(i, pipes);
		close(pipes[i][0]);  // cierro el pipe de lectura
	}

	// Distribuir archivos iniciales entre los esclavos
	for (int i = 0; i < SLAVES_NUM; i++) {
		write(pipes[i][1], FILE_PATHS[i], strlen(FILE_PATHS[i]) + 1);
	}

	for (int i = 0; i < SLAVES_NUM; i++) {
		close(pipes[i][1]);
	}
	int count = 0;
	while (count < SLAVES_NUM) {
		wait(NULL);
		printf("Waiting \n");
		count++;
	}

	return 0;
}

void prepare_slave(int id, int pipes[SLAVES_NUM][2]) {
	printf("creating slave %d \n", id);
	pid_t pid = fork();

	if (pid == -1) {
		perror("Error al crear proceso esclavo");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		// Código del proceso esclavo

		close(pipes[id][1]);  // Cerramos el extremo de escritura no utilizado

		char buf[256];  // Tamaño máximo del nombre del archivo

		read(pipes[id][0], buf, sizeof(buf));  // busca el primer archivo

		printf("Ejecutando Slave \n");

		char* slave[] = {"slave", buf, NULL};
		char* env[] = {NULL};

		execve("./slave", slave, NULL);

		perror("error Creating slave");
		exit(EXIT_FAILURE);
	}
}

void create_pipes(int pipes[SLAVES_NUM][2]) {
	for (int i = 0; i < SLAVES_NUM; i++) {
		/* code */
		if ((pipe(pipes[i])) == -1) {
			perror("error al crear pipes");
			exit(EXIT_FAILURE);
		}
		printf("succesfully created pipe %d \n", i);
	}
}
