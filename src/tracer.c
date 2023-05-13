#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#include "../includes/program.h"


int numNums(int num) {
    int numNums = 1;

    while(num/10 != 0){
        numNums++;
        num /= 10;
    }

    return numNums;
}

void reverse(char* str) {
    int i, j;
    char c;

    for (i = 0, j = strlen(str)-1; i<j; i++, j--) {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

void itoa(int n, char* str){
    int i = 0;

    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    str[i] = '\0';

    reverse(str);
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

    program.program_name = tokens[0];

    program.argc = num_tokens;

    program.argv = (char**)malloc(sizeof(char*) * (program.argc + 1));

    for(int i = 0; i < num_tokens; i++){
        program.argv[i] = tokens[i];
    }

    program.argv[program.argc] = NULL;

    free(tokens);

    return program;
}

int* pidsParser(int argc, char** argv, int start) {
    int pid = 0, numPids = 0;
    int* pidsList = malloc(sizeof(int) * argc); // aloca um array com o tamanho máximo possível

    char* token = NULL;

    for (int i = start; argv[i]; i++) {
        token = strtok(argv[i], " ");
        while(token != NULL){
            pid = atoi(token);
            pidsList[numPids] = pid;
            numPids += 1;
            token = strtok(NULL, " ");
        }
    }

    pidsList = realloc(pidsList, sizeof(int) * numPids); // redimensiona o array para o tamanho atual

    return pidsList;
}


long sendInitialStatus(int pid, char* program_name) {
    int client_server = open("client_server_fifo", O_WRONLY, 0666);
    if (client_server == -1) {
        perror("Error opening client_server_fifo\n");
        _exit(1);
    }

    int flag = 1;
    if (write(client_server, &flag, sizeof(flag)) == -1) {
        perror("Error sending flag\n");
        _exit(1);
    }

    if (write(client_server, &pid, sizeof(pid)) == -1) {
        perror("Error sending pid\n");
        _exit(1);
    }

    int len = strlen(program_name);

    if (write(client_server, &len, sizeof(len)) == -1) {
        perror("Error sending program name len\n");
        _exit(1);
    }

    if (write(client_server, program_name, strlen(program_name)) == -1) {
        perror("Error sending program name\n");
        _exit(1);
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long timestampI = current_time.tv_sec * 1000 + current_time.tv_usec / 1000; // milisegundos

    if (write(client_server, &timestampI, sizeof(timestampI)) == -1) {
        perror("Error sending initial timestamp\n");
        _exit(1);
    }

    close(client_server);

    return timestampI;
}


long sendFinalStatus(int pid) {
    int client_server = open("client_server_fifo", O_WRONLY, 0666);
    if (client_server == -1) {
        perror("Error opening client_server_fifo\n");
        _exit(1);
    }

    int flag = 2;
    if (write(client_server, &flag, sizeof(flag)) == -1) {
        perror("Error sending flag\n");
        _exit(1);
    }

    if (write(client_server, &pid, sizeof(pid)) == -1) {
        perror("Error sending pid\n");
        _exit(1);
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long timestampF = current_time.tv_sec * 1000 + current_time.tv_usec / 1000; // milisegundos

    if (write(client_server, &timestampF, sizeof(timestampF)) == -1) {
        perror("Error sending final timestamp\n");
        _exit(1);
    }

    close(client_server);

    return timestampF;
}

void stats_command(char* prog_name, int* pidsList, int size) {

    int client_server = open("client_server_fifo", O_WRONLY, 0666);
    if (client_server == -1){
        perror("Error opening client_server_fifo\n");
    }

    int flag = 5;
    if (write(client_server, &flag, sizeof(flag)) == -1){
        perror("Error sending flag\n");
        _exit(1);
    }

    int len = strlen(prog_name);

    if (write(client_server, &len, sizeof(len)) == -1){
        perror("Error sending name_len\n");
        _exit(1);
    }

    if (write(client_server, prog_name, strlen(prog_name)) == -1){
        perror("Error sending name_len\n");
        _exit(1);
    }

    if (write(client_server, &size, sizeof(size)) == -1){
        perror("Error sending array size\n");
        _exit(1);
    }

    for (int i = 0; i < size; i++) {
        if (write(client_server, &pidsList[i], sizeof(pidsList[i])) == -1) {
            perror("Error sending pid\n");
            _exit(1);
        }
    }

    int pid = getpid();
    if (write(client_server, &pid, sizeof(pid)) == -1) {
        perror("Error sending pid\n");
        _exit(1);
    }

    char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
    itoa(pid, pid_str);
    char* fifo;
    fifo = (char*)malloc(sizeof("server_client_fifo") + sizeof(pid_str));
    strcpy(fifo, "server_client_fifo_");
    strcat(fifo, pid_str);

    close(client_server);

    if (mkfifo(fifo, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            _exit(1);
        }
    }

    int server_client = open(fifo, O_RDONLY, 0666);
    if (client_server == -1) {
        perror("Error opening server_client_fifo\n");
        _exit(1);
    }

    int res;
    if (read(server_client, &res, sizeof(res)) == -1) {
        perror("Error reading result");
        _exit(1);
    }

    char* res_str = (char*)malloc(sizeof(char) * numNums(res));;
    itoa(res, res_str);

    char* msg = (char*)malloc(sizeof(prog_name) + sizeof(" was executed ") + sizeof(res_str) + sizeof(" times\n"));

    strcpy(msg, prog_name);
    strcat(msg, " was executed ");
    strcat(msg, res_str);
    strcat(msg, " times\n");

    write(1, msg, strlen(msg));

    close(server_client);

    unlink(fifo);

}

void stats_uniq(int* pidsList, int size) {

    int client_server = open("client_server_fifo", O_WRONLY, 0666);
    if (client_server == -1) {
        perror("Error opening client_server_fifo\n");
        _exit(1);
    }

    int flag = 6;
    if (write(client_server, &flag, sizeof(flag)) == -1) {
        perror("Error sending flag\n");
        _exit(1);
    }

    if (write(client_server, &size, sizeof(size)) == -1){
        perror("Error sending array size\n");
        _exit(1);
    }

    for (int i = 0; i < size; i++) {
        if (write(client_server, &pidsList[i], sizeof(pidsList[i])) == -1) {
            perror("Error sending pid\n");
            _exit(1);
        }
    }

    int pid = getpid();
    if (write(client_server, &pid, sizeof(pid)) == -1) {
        perror("Error sending pid\n");
        _exit(1);
    }

    close(client_server);

    char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
    itoa(pid, pid_str);
    char* fifo;
    fifo = (char*)malloc(sizeof("server_client_fifo") + sizeof(pid_str));
    strcpy(fifo, "server_client_fifo_");
    strcat(fifo, pid_str);

    if (mkfifo(fifo, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            _exit(1);
        }
    }

    int server_client = open(fifo, O_RDONLY, 0666);
    if (client_server == -1) {
        perror("Error opening server_client_fifo\n");
        _exit(1);
    }

    int new_size;
    if (read(server_client, &new_size, sizeof(new_size)) == -1) {
        perror("Error reading array size\n");
        _exit(1);
    }

    for (int i = 0; i < new_size; i++) {
        int str_len;
        if (read(server_client, &str_len, sizeof(str_len)) == -1) {
            perror("Error reading program name length\n");
            _exit(1);
        }

        char* string = (char*)malloc(sizeof(char) * (str_len + 1));
        if (read(server_client, string, str_len) == -1) {
            perror("Error reading program name\n");
            _exit(1);
        }
        string[str_len] = '\0';
        // printf("prog_name: %s\n", string);

        write(1, string, strlen(string));
        write(1, "\n", strlen("\n"));
    }

    close(server_client);

    unlink(fifo);

}

int ficheiroTxt(char* pid_str, char* program_name, char* exec_time, char* caminho){

    char* filename = (char*)malloc(sizeof(caminho) + sizeof(pid_str) + sizeof(".txt"));
    strcpy(filename, caminho);
    strcat(filename, pid_str);
    strcat(filename, ".txt");

    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if(fd){
        char* ficheiro = (char*)malloc( sizeof("PID: ") + sizeof(pid_str) + sizeof("\nProgram name: ") + sizeof(program_name) + sizeof("\nExec_time: ") + sizeof(exec_time) + sizeof(" ms\n"));
        strcpy(ficheiro, "PID: ");
        strcat(ficheiro, pid_str);
        strcat(ficheiro, "\nProgram name: ");
        strcat(ficheiro, program_name);
        strcat(ficheiro, "\nExec_time: ");
        strcat(ficheiro, exec_time);
        strcat(ficheiro, " ms");

        int len = strlen(ficheiro);


        if(write(fd, ficheiro, len) == -1){
            perror("Error writing string\n");
            _exit(1);
        }

    }else{
        perror("Could not create the file\n");
        _exit(1);
    }
    close(fd);
}


int main(int argc, char **argv) {

    if (argc > 1) {
        if (strcmp(argv[1], "execute") == 0) {
            if (argc < 4) {
                perror("Insufficient arguments\n");
                return 0;
            }

            if (strcmp(argv[2], "-u") == 0) {

                int status;
                Program program = parser(argc, argv);

                int fd[2];
                if (pipe(fd) == -1) {
                    perror("Error creating pipe\n");
                    _exit(1);
                }

                if (fork() == 0) {

                    close(fd[0]);

                    int pid = getpid();
                    if (write(fd[1], &pid, sizeof(pid)) == -1) {
                        perror("FILHO | Error sending pid\n");
                        _exit(1);
                    }

                    close(fd[1]);

                    execvp(program.program_name, program.argv);

                } else {

                    close(fd[1]);

                    int pid;
                    if (read(fd[0], &pid, sizeof(pid)) == -1) {
                        perror("PAI | Error reading pid\n");
                        _exit(1);
                    }

                    close(fd[0]);

                    program.process_pid = pid;
                    char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
                    itoa(pid, pid_str);

                    long timestampI = sendInitialStatus(program.process_pid, program.program_name);

                    char* msg1 = (char*)malloc(sizeof("Running PID ") + sizeof(pid_str) + sizeof("\n"));
                    strcpy(msg1, "Running PID ");
                    strcat(msg1, pid_str);
                    strcat(msg1, "\n");
                    write(1, msg1, strlen(msg1));

                    wait(&status);

                    long timestampF = sendFinalStatus(program.process_pid);

                    int exec_time = timestampF - timestampI;
                    char* exec_time_str = (char*)malloc(sizeof(char) * numNums(exec_time));
                    itoa(exec_time, exec_time_str);

                    char* msg2 = (char*)malloc(sizeof("Ended in ") + sizeof(exec_time_str) + sizeof(" ms\n"));
                    strcpy(msg2, "Ended in ");
                    strcat(msg2, exec_time_str);
                    strcat(msg2, " ms\n");
                    write(1, msg2, strlen(msg2));

                    if (argc == 5){
                        char* caminho = argv[4];
                        ficheiroTxt(pid_str, program.program_name, exec_time_str, caminho);
                    }
                }


            } else if (strcmp(argv[2], "-p") == 0) {

            }

        } else if (strcmp(argv[1], "status") == 0) {
            if (argc != 2) {
                perror("Insufficient arguments\n");
                return 0;
            }

            int client_server = open("client_server_fifo", O_WRONLY, 0666);
            if (client_server == -1) {
                perror("Error opening client_server_fifo\n");
                _exit(1);
            }

            int flag = 3;
            if (write(client_server, &flag, sizeof(flag)) == -1) {
                perror("Error sending flag\n");
                _exit(1);
            }

            int pid = getpid();
            if (write(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error sending pid\n");
                _exit(1);
            }

            char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
            itoa(pid, pid_str);

	        char* fifo;
            fifo = (char*)malloc(sizeof("server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "server_client_fifo_");
	        strcat(fifo, pid_str);

            close(client_server);

	        if (mkfifo(fifo, 0666) == -1) {
                if (errno != EEXIST) {
                    perror("Could not create client_server_fifo\n");
                    _exit(1);
                }
            }

            int server_client = open(fifo, O_RDONLY, 0666);
            if (client_server == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            int num_processes;
            if (read(server_client, &num_processes, sizeof(num_processes)) == -1) {
                perror("Error reading num_processes\n");
                _exit(1);
            }

            char* string;
            for (int i = 0; i < num_processes; i++) {
                int len;
                if (read(server_client, &len, sizeof(len)) == -1) {
                    perror("Error reading length\n");
                    _exit(1);
                }

                string = (char*)malloc(sizeof(char*) * len);

                if (read(server_client, string, len) == -1) {
                    perror("Error reading string\n");
                    _exit(1);
                }

                printf("%s\n", string);
                // write(1, string, strlen(string));
            }

            close(server_client);

            unlink(fifo);

        } else if (strcmp(argv[1], "stats-time") == 0) {
            if (argc < 3){
                perror("Insufficient arguments\n");
                _exit(1);
            }

            int* pidsList = (int*)malloc(sizeof(int) * (argc-2));
            pidsList = pidsParser(argc, argv, 2);
            int len = argc-2;

            for (int i = 0; i < len; i++) {
                printf("pid-%d: %d\n", i, pidsList[i]);
            }

            int client_server = open("client_server_fifo", O_WRONLY, 0666);
            if (client_server == -1) {
                perror("Error opening client_server_fifo\n");
                _exit(1);
            }

            int flag = 4;
            if (write(client_server, &flag, sizeof(flag)) == -1) {
                perror("Error sending flag\n");
                _exit(1);
            }

            if (write(client_server, &len, sizeof(len)) == -1) {
                perror("Error sending len\n");
                _exit(1);
            }

            for (int i = 0; i < len; i++) {
                if (write(client_server, &pidsList[i], sizeof(pidsList[i])) == -1) {
                    perror("Error sending pid\n");
                    return 0;
                }
            }

            int pid = getpid();
            char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
            itoa(pid, pid_str);
	        char* fifo;
            fifo = (char*)malloc(sizeof("server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "server_client_fifo_");
	        strcat(fifo, pid_str);

	        if (mkfifo(fifo, 0666) == -1) {
                if (errno != EEXIST) {
                    perror("Could not create server_client_fifo\n");
                    _exit(1);
                }
            }

            if (write(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error sending pid\n");
                _exit(1);
            }

            close(client_server);

            int server_client = open(fifo, O_RDONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            int sum;
            if (read(server_client, &sum, sizeof(sum)) == -1) {
                perror("Error reading sum\n");
                _exit(1);
            }

            char* sum_str = (char*)malloc(sizeof(char) * numNums(sum));;
            itoa(sum, sum_str);

            char* msg = (char*)malloc(sizeof("Total execution time is ") + sizeof(sum_str) + sizeof(" ms\n"));

            strcpy(msg, "Total execution time is ");
            strcat(msg, sum_str);
            strcat(msg, " ms\n");

            write(1, msg, strlen(msg));

            close(server_client);

            unlink(fifo);

        } else if (strcmp(argv[1], "stats-command") == 0) {
            if (argc < 3){
                perror("Insufficient arguments\n");
                return 0;
            }

            char* prog = (char*)malloc(sizeof(char) * strlen(argv[2]));
            strcpy(prog, argv[2]);

            int* pidsList = malloc(sizeof(int) * (argc-3));
            pidsList = pidsParser(argc, argv, 3);

            int size = argc-3;

            stats_command(prog, pidsList, size);

        } else if (strcmp(argv[1], "stats-uniq") == 0){
            if (argc < 3) {
                perror("Insufficient arguments\n");
                return 0;
            }

            int* pidsList = (int*)malloc(sizeof(int) * (argc-2));
            pidsList = pidsParser(argc, argv, 2);

            int size = argc-2;

            stats_uniq(pidsList, size);

        }
    }

    return 0;
}
