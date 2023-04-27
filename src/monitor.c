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

char* itoa(int val, int base){

	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}



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
        //         receiveNewProcess();
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
            // printf("timestampI: %ld\n", list[n].timestampI);
            // printf("timestampF: %ld\n", list[n].timestampF);
            printf("exec_time: %d\n\n", list[n].exec_time);

        } else if (flag == 3) {

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            close(client_server);

	        char* pid_str = itoa(pid, 10);
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

            // printf("num_processes: %d\n", num_processes);
            // for (int i = 0; i < num_processes; i++) {
            //     printf("i: %d\n", i);
            //     printf("pid: %d\n", list[i].process_pid);
            //     printf("prog_name: %s\n", list[i].program_name);
            //     printf("exec_time: %d\n", list[i].exec_time);
            // }

            char* buffer; // PID prog_name exec_time
            for (int i = 0; i < num_processes; i++) {
                buffer = NULL;
                int pid = list[i].process_pid;
                printf("pid: %d\n", pid);
                char* pid_str = itoa(pid, 10);
                printf("pid_str: %s\n", pid_str);

                int exec_time = list[i].exec_time;
                // printf("exec_time: %d\n", exec_time);
                char* exec_time_str = itoa(exec_time, 10);
                // printf("exec_time_str: %s\n", exec_time_str);

                buffer = (char*)malloc(sizeof(pid_str) + sizeof(" ") + sizeof(list[i].program_name) + sizeof(" ") + sizeof(exec_time_str) + sizeof(" ms\n"));
                printf("1 - %s\n", buffer);

                strcpy(buffer, pid_str);
                printf("2- %s\n", buffer);
                strcat(buffer, " ");
                strcat(buffer, list[i].program_name);
                printf("3 - %s\n", buffer);
                strcat(buffer, " ");
                strcat(buffer, exec_time_str);
                printf("4 - %s\n", buffer);
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
