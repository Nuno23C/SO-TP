#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

void receiveRequest(int agrc, char **argv) {
    int server_client_fifo = open("server_client_fifo", O_WRONLY | O_TRUNC, 0666);
    if (server_client_fifo == -1) {
        perror("Error opening server_client_fifo\n");
        _exit(1);
    }

    int client_server_fifo = open("client_server_fifo", O_RDONLY, 0666);
    if (client_server_fifo == -1) {
        perror("Error opening client_server_fifo\n");
        _exit(1);
    }
}

int main(int agrc, char **argv) {
    if (mkfifo("server_client_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            return 1;
        }
    }

    receiveRequest(agrc, argv);
}
