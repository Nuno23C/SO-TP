#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#include "../includes/status.h"
#include "../includes/program.h"

#define SIZE 1024

char* itoa(int val, int base){

	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}

void sendInitialStatus(int fifo) {
    Status status;

    int pid = getpid();
    status.process_pid = pid;

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    status.timestampI = current_time.tv_sec;

    write(fifo, &status, sizeof(Status));

    _exit(0);
}

void receiveStatus() {
    int server_client = open("server_client_fifo", O_RDONLY, 0666);
    if (server_client == -1) {
        perror("Error opening server_client_fifo\n");
        _exit(1);
    }

    char buffer[SIZE];
    int bytesRead;

    while (bytesRead = read(server_client, buffer, SIZE) > 0) {
        write(1, buffer, bytesRead);
    }

    close(server_client);

    _exit(0);
}

Program parser(int argc, char** argv) {
    char* token = strtok(argv[3], " ");
    char** tokens = NULL;
    int num_tokens = 0;

    while (token) {
        tokens = realloc(tokens, (num_tokens + 1) * sizeof(char*));
        tokens[num_tokens++] = strdup(token);
        token = strtok(NULL, " ");
    }

    Program program;

    int pid = getpid();
    program.process_pid = pid;

    program.program_name = tokens[0];

    program.argc = argc - 1;

    for (int i = 1; i < num_tokens; i++) {
        strcpy(program.argv[i - 1], tokens[i]);
    }

    return program;
}

int main(int argc, char **argv) {
    if (mkfifo("client_server_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create client_server_fifo\n");
            _exit(1);
        }
    }

    if (argc > 1) {
        if (strcmp(argv[1], "execute") == 0) {
            if (argc < 4) {
                perror("Insufficient arguments\n");
                return 0;
            }

            // int client_server = open("client_server_fifo", O_WRONLY, 0666);
            // if (client_server == -1) {
            //     perror("Error opening client_server_fifo\n");
            //     _exit(1);
            // }

            if (strcmp(argv[2], "-u") == 0) {
                Program program = parser(argc, argv);
                printf("Running PID %d\n", program.process_pid);
                // sendStatus(client_server);
                execvp(program.program_name, program.argv);
                printf("here\n");


                // printf("program_name: %s\n", program.program_name);
                // printf("argc: %d\n", program.argc);
                // printf("argv: ");
                // for (int i = 0; i < program.argc; i++) {
                //     printf("%s ", program.argv[i]);
                // }
            } else if (strcmp(argv[2], "-p") == 0) {
                // for ...
            }

            // close(client_server);

        } else if (strcmp(argv[1], "status") == 0) {
            if (argc != 2) {
                perror("Insufficient arguments\n");
                return 0;
            }

        }

    }

    // receiveResponse();

    unlink("client_server_fifo");

    return 0;
}
