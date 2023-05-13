#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../includes/process.h"

Process* list;
int num_processes = 0;

Process* active_list;
int active_processes = 0;

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

void remove_process_by_pid(int pid) {
    int i, j;
    for (i = 0; i < active_processes; i++) {
        if (active_list[i].process_pid == pid) {
            free(active_list[i].program_name);
            for (j = i; j < active_processes - 1; j++) {
                active_list[j] = active_list[j+1]; // move os elementos subsequentes uma posição para trás
            }
            active_processes--;
            active_list = realloc(active_list, active_processes * sizeof(Process));
            break;
        }
    }
}

void remove_duplicate_strings(char** prog_names, int size) {
    int i, j, k;

    // Percorre todas as strings do array
    for (i = 0; i < size; i++) {
        // Verifica se a string na posição i já foi verificada
        if (prog_names[i] != NULL) {
            // Percorre as strings a partir da posição i + 1
            for (j = i + 1; j < size; j++) {
                // Verifica se as strings são iguais
                if (prog_names[j] != NULL && strcmp(prog_names[i], prog_names[j]) == 0) {
                    // Remove a string duplicada, movendo todas as strings seguintes uma posição para trás
                    free(prog_names[j]);
                    prog_names[j] = NULL;
                }
            }
        }
    }

    // Removendo as posições NULL do array
    for (i = 0, k = 0; i < size; i++) {
        if (prog_names[i] != NULL) {
            prog_names[k++] = prog_names[i];
        }
    }

    prog_names[k] = NULL;
}

int main(int argc, char **argv){

    if (mkfifo("../tmp/client_server_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create client_server_fifo\n");
            _exit(1);
        }
    }

    while(1) {

        int client_server = open("../tmp/client_server_fifo", O_RDONLY, 0666);
        if (client_server == -1){
            perror("Could not open the fifo client_server\n");
            _exit(1);
        }


        int flag;
        if (read(client_server, &flag, sizeof(flag)) == -1) {
            perror("Error reading flag");
            _exit(1);
        }

        if (flag == 1) { // NOVO PROGRAMA

            Process process;

            if (read(client_server, &process.process_pid, sizeof(process.process_pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            int program_name_len;
            if (read(client_server, &program_name_len, sizeof(program_name_len)) == -1) {
                perror("Error reading program name length\n");
                _exit(1);
            }

            process.program_name = (char*)malloc(sizeof(char) * program_name_len);

            if (read(client_server, process.program_name, sizeof(char) * program_name_len) == -1) {
                perror("Error reading program name\n");
                _exit(1);
            }
            process.program_name[program_name_len] = '\0';

            // printf("program_name: %s\n", process.program_name);

            if (read(client_server, &process.timestampI, sizeof(process.timestampI)) == -1) {
                perror("Error reading from tracer\n");
                _exit(1);
            }

            //adiciona um novo processo à lista de processos ativos
            active_processes++;
            active_list = realloc(active_list, active_processes * sizeof(Process));
            active_list[active_processes - 1] = process;

            close(client_server);

        } else if (flag == 2) { // FIM DE UM PROGRAMA

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            int n;
            for (int i = 0; i < active_processes; i++) {
                if (active_list[i].process_pid == pid) {
                    n = i;
                    break;
                }
            }

            if (read(client_server, &active_list[n].timestampF, sizeof(active_list[n].timestampF)) == -1) {
                perror("Error reading final timestamp\n");
                _exit(1);
            }

            active_list[n].exec_time = active_list[n].timestampF - active_list[n].timestampI;

            int str_len = strlen(active_list[n].program_name);

            //adiciona o processo à lista de processos terminados
            num_processes++;
            list = realloc(list, num_processes * sizeof(Process));
            list[num_processes - 1].program_name = (char*)malloc(sizeof(char) * str_len);
            list[num_processes - 1].process_pid = active_list[n].process_pid;
            strcpy(list[num_processes - 1].program_name, active_list[n].program_name);
            list[num_processes - 1].exec_time = active_list[n].exec_time;

            //remove o processo da lista de processos ativos
            remove_process_by_pid(pid);

            printf("pid: %d\n", list[num_processes - 1].process_pid);
            printf("program_name: %s\n", list[num_processes - 1].program_name);
            printf("exec_time: %d\n\n", list[num_processes - 1].exec_time);

            close(client_server);

        } else if (flag == 3) { // STATUS

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1) {
                perror("Error reading pid\n");
                _exit(1);
            }

            close(client_server);

	        char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
            itoa(pid, pid_str);
	        char* fifo;
            fifo = (char*)malloc(sizeof("../tmp/server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "../tmp/server_client_fifo_");
	        strcat(fifo, pid_str);

            while (access(fifo, F_OK) == -1) {
                // caso o fifo não tenha sido criado ainda, espera 1 segundo
                sleep(1);
            }

            int server_client = open(fifo, O_WRONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            if (write(server_client, &num_processes, sizeof(num_processes)) == -1) {
                perror("Error writing num_processes\n");
                _exit(1);
            }

            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            long current_time_ms = current_time.tv_sec * 1000 + current_time.tv_usec / 1000; // milisegundos

            char* buffer;
            for (int i = 0; i < active_processes; i++) {

                int pid = active_list[i].process_pid;
                char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
                itoa(pid, pid_str);

                int exec_time = current_time_ms - active_list[i].timestampI;
                char* exec_time_str = (char*)malloc(sizeof(char) * numNums(exec_time));
                itoa(exec_time, exec_time_str);

                buffer = (char*)malloc(sizeof(pid_str) + sizeof(" ") + sizeof(active_list[i].program_name) + sizeof(" ") + sizeof(exec_time_str) + sizeof(" ms\n"));

                strcpy(buffer, pid_str);
                strcat(buffer, " ");
                strcat(buffer, active_list[i].program_name);
                strcat(buffer, " ");
                strcat(buffer, exec_time_str);
                strcat(buffer, " ms");

                int len = strlen(buffer);

                // printf("Vai enviar: %s\n", buffer);

                if (write(server_client, &len, sizeof(len)) == -1) {
                    perror("Error writing string length\n");
                    _exit(1);
                }

                if (write(server_client, buffer, len) == -1) {
                    perror("Error writing string\n");
                    _exit(1);
                }

            }

            // printf("-----------------------\n");
            // printf("Na lista de todos os processos:\n");
            // for (int i = 0; i < num_processes; i++) {
            //     printf("| i = %d |\n", i);
            //     printf("pid: %d\n", list[i].process_pid);
            //     printf("program_name: %s\n", list[i].program_name);
            //     printf("exec_time: %d\n", list[i].exec_time);
            // }
            // printf("-----------------------\n\n");
            // printf("-----------------------\n");
            // printf("Na lista de processos ativos:\n");
            // for (int i = 0; i < active_processes; i++) {
            //     printf("| i = %d |\n", i);
            //     printf("pid: %d\n", active_list[i].process_pid);
            //     printf("program_name: %s\n", active_list[i].program_name);
            //     printf("exec_time: %d\n", active_list[i].exec_time);
            // }
            // printf("-----------------------\n");

            close(server_client);

        } else if (flag == 4) { // STATS-TIME

            int len;
            if (read(client_server, &len, sizeof(len)) == -1){
                perror("Error reading len\n");
                _exit(1);
            }

            int sum = 0;
            for (int i = 0; i < len; i++) {
                int pid;
                if(read(client_server, &pid, sizeof(pid)) == -1){
                    perror("Error reading pid\n");
                }
                //percorrer a lista de processos terminados
                for (int i = 0; i < num_processes; i++) {
                    if (list[i].process_pid == pid) {
                        sum += list[i].exec_time;
                    }
                }
            }

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1){
                perror("Error reading pid\n");
                _exit(1);
            }

            close(client_server);

            char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
            itoa(pid, pid_str);
            char* fifo;
            fifo = (char*)malloc(sizeof("../tmp/server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "../tmp/server_client_fifo_");
	        strcat(fifo, pid_str);

            while (access(fifo, F_OK) == -1) {
                // caso o fifo não tenha sido criado ainda, espera 1 segundo
                sleep(1);
            }

            int server_client = open(fifo, O_WRONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            if (write(server_client, &sum, sizeof(sum)) == -1) {
                perror("Error sending sum\n");
                _exit(1);
            }

            close(server_client);

        } else if (flag == 5) { // STATS-COMMAND

            int len;
            if (read(client_server, &len, sizeof(len)) == -1) {
                perror("Error reading length\n");
                _exit(1);
            }

            char* prog_name = (char*)malloc(sizeof(char) * (len + 1));
            if (read(client_server, prog_name, sizeof(char) * len) == -1) {
                perror("Error reading name\n");
                _exit(1);
            }
            prog_name[len] = '\0';

            int size;
            if (read(client_server, &size, sizeof(size)) == -1) {
                perror("Error reading array size\n");
                _exit(1);
            }

            int sum = 0;
            int pid;
            for (int i = 0; i < size; i++) {
                if (read(client_server, &pid, sizeof(int)) == -1) {
                    perror("Error reading pid\n");
                    _exit(1);
                }
                for (int j = 0; j < num_processes; j++) {
                    if ((pid == list[j].process_pid) && (strcmp(list[j].program_name, prog_name) == 0)) {
                        sum ++;
                        break;
                    }
                }
            }

            int pid_;
            if (read(client_server, &pid_, sizeof(pid_)) == -1) {
                    perror("Error reading pid\n");
                    _exit(1);
            }

            close(client_server);

	        char* pid_str = (char*)malloc(sizeof(char) * numNums(pid_));
            itoa(pid_, pid_str);
	        char* fifo;
            fifo = (char*)malloc(sizeof("../tmp/server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "../tmp/server_client_fifo_");
	        strcat(fifo, pid_str);

            while (access(fifo, F_OK) == -1) {
                // caso o fifo não tenha sido criado ainda, espera 1 segundo
                sleep(1);
            }

            int server_client = open(fifo, O_WRONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            if (write(server_client, &sum, sizeof(sum)) == -1) {
                perror("Error sending result");
                _exit(1);
            }

            close(server_client);

        } else if (flag == 6) { // STATS-UNIQ

            int size;
            if (read(client_server, &size, sizeof(size)) == -1) {
                perror("Error reading array size\n");
                _exit(1);
            }

            char** prog_names = (char**)malloc(sizeof(char*) * size);
            for (int i = 0; i < size; i++) {
                int pid;
                if(read(client_server, &pid, sizeof(pid)) == -1){
                    perror("Error reading pid\n");
                    _exit(1);
                }

                for (int j = 0; j < num_processes; j++) {
                    if (list[j].process_pid == pid) {
                        prog_names[i] = (char*)malloc(sizeof(char) * strlen(list[j].program_name));
                        strcpy(prog_names[i], list[j].program_name);
                        break;
                    }
                }
            }

            remove_duplicate_strings(prog_names, size);

            int new_size = 0;
            char** p = prog_names;
            while (*p) {
                new_size++;
                p++;
            }

            int pid;
            if (read(client_server, &pid, sizeof(pid)) == -1){
                perror("Error reading pid\n");
                _exit(1);
            }

            close(client_server);

            char* pid_str = (char*)malloc(sizeof(char) * numNums(pid));
            itoa(pid, pid_str);
            char* fifo;
            fifo = (char*)malloc(sizeof("../tmp/server_client_fifo_") + sizeof(pid_str));
            strcpy(fifo, "../tmp/server_client_fifo_");
	        strcat(fifo, pid_str);

            while (access(fifo, F_OK) == -1) {
                // caso o fifo não tenha sido criado ainda, espera 1 segundo
                sleep(1);
            }

            int server_client = open(fifo, O_WRONLY, 0666);
            if (server_client == -1) {
                perror("Error opening server_client_fifo\n");
                _exit(1);
            }

            if (write(server_client, &new_size, sizeof(new_size)) == -1) {
                perror("Error sending array size\n");
                _exit(1);
            }

            for (int i = 0; i < new_size; i++) {
                int str_len = strlen(prog_names[i]);
                if (write(server_client, &str_len, sizeof(str_len)) == -1) {
                    perror("Error sending program name length\n");
                    _exit(1);
                }

                if (write(server_client, prog_names[i], str_len) == -1) {
                    perror("Error sending program name\n");
                    _exit(1);
                }
            }

            close(server_client);

        }
    }

    unlink("../tmp/client_server_fifo");

    return 0;
}
