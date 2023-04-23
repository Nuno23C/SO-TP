#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../includes/process.h"

#define SIZE 1024

int main(int argc, char **argv){
    int num_processes = 0;
    Process list[SIZE];

    if (mkfifo("client_server_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create client_server_fifo\n");
            _exit(1);
        }
    }

    // if (mkfifo("server_client_fifo", 0666) == -1) {
    //     if (errno != EEXIST) {
    //         perror("Could not create client_server_fifo\n");
    //         _exit(1);
    //     }
    // }

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
            printf("timestampI: %ld\n", list[n].timestampI);
            printf("timestampF: %ld\n", list[n].timestampF);
            printf("exec_time: %d\n\n", list[n].exec_time);

        }

        close(client_server);

        // int server_client = open("server_client_fifo", O_WRONLY, 0666);
        // if (server_client == -1) {
        //     perror("Error opening server_client_fifo\n");
        //     _exit(1);
        // }

        // char* string = "teste123";
        // int len = strlen(string);

        // if (write(server_client, &len, sizeof(len)) == -1) {
        //     perror("Error wtring string length\n");
        //     _exit(1);
        // }

        // if (write(server_client, string, len) == -1) {
        //     perror("Error wtring string\n");
        //     _exit(1);
        // }

        // close(server_client);
    }

    unlink("client_server_fifo");
    // unlink("server_client_fifo");


    return 0;
}
