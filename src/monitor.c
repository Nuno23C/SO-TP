#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../includes/process.h"

#define SIZE 1024

Process list[SIZE];
int num_processes = 0;

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

// void newProcessReceived(int fifo) {
// }

int main(int argc, char **argv){

    if (mkfifo("client_server_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create client_server_fifo\n");
            _exit(1);
        }
    }

    while(1) {

        int client_server = open("client_server_fifo", O_RDONLY, 0666);
        if (client_server == -1){
            perror("Could not open the fifo client_server\n");
            _exit(1);
        }

        Process process;

        int flag;
        if (read(client_server, &flag, sizeof(flag)) == -1) {
            perror("Error reading flag");
            _exit(1);
        }

        // switch(flag) {
        //     case 1:
        //         newProcessReceived(client_server);
        //         break;

        //     case 2:
        //         endOfProcess(client_server, pid);
        // }

        if (flag == 1) { // Vai receber um novo programa

            if (read(client_server, &process.process_pid, sizeof(process.process_pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }
            // printf("process_pid: %d\n", process.process_pid);

            int program_name_len;
            if (read(client_server, &program_name_len, sizeof(program_name_len)) == -1) {
                perror("Error reading program name length\n");
                _exit(1);
            }

            process.program_name = (char*)malloc(program_name_len);

            if (read(client_server, process.program_name, program_name_len) == -1) {
                perror("Error reading program name\n");
                _exit(1);
            }
            // printf("program_name: %s\n", process.program_name);

            if (read(client_server, &process.timestampI, sizeof(process.timestampI)) == -1) {
                perror("Error reading from tracer\n");
                _exit(1);
            }
            // printf("timestampI: %ld\n", process.timestampI);

            list[num_processes] = process;
            num_processes += 1;

        } else if (flag == 2) { // vai receber o fim de um programa

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            int n;
            for (int i = 0; i < num_processes; i++) {
                if (list[i].process_pid == pid) {
                    n = i;
                }
            }

            if (read(client_server, &list[n].timestampF, sizeof(process.timestampF)) == -1) {
                perror("Error reading from tracer\n");
                _exit(1);
            }

            list[n].exec_time = list[n].timestampF - list[n].timestampI;

            printf("list_indice: %d\n", n);
            printf("process_pid: %d\n", list[n].process_pid);
            printf("program_name: %s\n", list[n].program_name);
            printf("exec_time: %d\n\n", list[n].exec_time);

        } else if (flag == 3) {

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            close(client_server);

	        char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
	        char* fifo;
            fifo = (char*)malloc(sizeof("server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "server_client_fifo_");
	        strcat(fifo, pid_str);

            int server_client = open(fifo, O_WRONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            if (write(server_client, &num_processes, sizeof(num_processes)) == -1) {
                perror("Error writing num_processes\n");
                _exit(1);
            }

            char* buffer;
            for (int i = 0; i < num_processes; i++) {

                int pid = list[i].process_pid;
                char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
                itoa(pid, pid_str);

                int exec_time = list[i].exec_time;
                char* exec_time_str = (char*)malloc(sizeof(char) * numNums(exec_time));
                itoa(exec_time, exec_time_str);

                buffer = (char*)malloc(sizeof(pid_str) + sizeof(" ") + sizeof(list[i].program_name) + sizeof(" ") + sizeof(exec_time_str) + sizeof(" ms\n"));

                strcpy(buffer, pid_str);
                strcat(buffer, " ");
                strcat(buffer, list[i].program_name);
                strcat(buffer, " ");
                strcat(buffer, exec_time_str);
                strcat(buffer, " ms");

                int len = strlen(buffer);

                printf("Vai enviar: %s\n", buffer);

                if (write(server_client, &len, sizeof(len)) == -1) {
                    perror("Error writing string length\n");
                    _exit(1);
                }

                if (write(server_client, buffer, len) == -1) {
                    perror("Error writing string\n");
                    _exit(1);
                }
            }

            close(server_client);
        }

        close(client_server);
    }

    unlink("client_server_fifo");

    return 0;
}
