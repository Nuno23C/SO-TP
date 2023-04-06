#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define SIZE 1024

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

void makeRequest(int argc, char **argv) {
    int client_server_fifo = open("client_server_fifo", O_WRONLY | O_TRUNC, 0666);
    if (client_server_fifo == -1) {
        perror("Error opening client_server_fifo\n");
        _exit(1);
    }

    if (mkfifo("server_client_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            return 1;
        }
    }

    int pid = getpid();
    char* pidClient = my_itoa(pid, 10);
    char buffer[SIZE];

    for (int i = 1; i < argc; i++) {
        strcat(buffer, *argv);
        srtcat(buffer, " ");
        argv++;
    }

    strcat(buffer, "");
    strcat(buffer, pidClient);

    print("CLIENT | vou escrever para o server");
    if (write(client_server_fifo, buffer, sifeof(buffer)) == -1) {
        perror("Error writing to server");
    }
    close(client_server_fifo);

    int server_client_fifo = open("server_client_fifo", O_RDONLY, 0666);
    if (server_client_fifo == -1) {
        perror("Error opening server_client_fifo\n");
        _exit(1);
    }

    char string[SIZE];
    int bytesRead;
    if (bytesRead = read(server_client_fifo, string, SIZE) == -1) {
        perror("Error reading from server");
    }

    while (bytesRead > 0) {
        write(1, string, bytesRead);
    }
    close(server_client_fifo);

    _exit(0);
}

int main(int agrc, char **argv) {
    if (mkfifo("client_server_fifo", 0777) == -1) {
        if (errno != EEXIST) {
            perror("Could not create client_server_fifo\n");
            return 1;
        }
    }

    makeRequest(agrc, argv);

    return 0;
}
