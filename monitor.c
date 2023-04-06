#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define SIZE 1024

/*
char* my_itoa(int value, int base) {
    char* result;

    // handle 0 explicitly
    if (value == 0) {
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    // handle negative numbers
    int is_negative = 0;
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    // convert the value to a string in reverse order
    int i = 0;
    while (value != 0) {
        int digit = value % base;
        result[i++] = digit < 10 ? digit + '0' : digit + 'A' - 10;
        value /= base;
    }

    // add the negative sign if necessary
    if (is_negative) {
        result[i++] = '-';
    }

    // add the null terminator and reverse the string
    result[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = result[j];
        result[j] = result[k];
        result[k] = temp;
    }

    return result;
}
*/
char* itoa(int val, int base){
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}


void receiveRequest(int argc, char **argv){
    char buffer1[SIZE]; //armezar informação
    char fifoName1[1024] = "server_client_fifo";

    int server_client = open(fifoName1, O_WRONLY, 0666);
    if((server_client = open(fifoName1, O_WRONLY, 0666)) == -1){
		perror("Could not open the fifo\n");
		_exit(1);
	}

    // Cria o pipe 
    if(mkfifo(fifoName1, 0666) == -1){
        if(errno != EEXIST){
            perror("Could not create fifo file\n");
            _exit(1);
        }
    }

    int pid = getpid();
    char* pidServer = itoa(pid, 10);
    argv++;

    for (int i = 1; i < argc; i++) {
        strcat(buffer1, *argv);
        strcat(buffer1, " ");
        argv++;
    }

    strcat(buffer1, "");
    strcat(buffer1, pidServer);


    printf("SERVER | vou escrever para o cliente");
    if (write(server_client, buffer1, sizeof(buffer1)) == -1) {
        perror("Error writing to server");
    }

    // Fecha o pipe nomeado
    close(server_client);


    int bytes_read = 0;
    char buffer2[SIZE];
    char fifoName2[1024] = "client_server_fifo";

    int client_server = open(fifoName2, O_RDONLY, 0666);
    if((client_server = open(fifoName2, O_RDONLY, 0666)) == -1){
		perror("Could not open the fifo\n");
		_exit(1);
	}

    while((bytes_read = read(client_server, buffer2, SIZE)) > 0){
		write(1, buffer2, bytes_read);
	}

    close(client_server);
}

int main(int argc, char **argv){
        if (mkfifo("server_client_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            _exit(1);
        }
    }

    receiveRequest(argc, argv);

    return 0;
}
