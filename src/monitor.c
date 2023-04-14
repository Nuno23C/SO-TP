#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../includes/status.h"

#define SIZE 1024

int main(int argc, char **argv){
    if(mkfifo("client_server_fifo", 0666) == -1){
        if(errno != EEXIST){
            perror("Could not create client_server_fifo\n");
            _exit(1);
        }
    }

    int client_server = open("client_server_fifo", O_RDONLY, 0666);
    if (client_server == -1){
		perror("Could not open the fifo\n");
		_exit(1);
	}

    // Request request;
    int bytesRead;

    // if ((bytesRead = read(client_server, &request, sizeof(Request))) == -1) {
    //     perror("Error reading from server");
    //     _exit(1);
    // }

    // if (mkfifo("server_client_fifo", 0666) == -1) {
    //     if (errno != EEXIST) {
    //         perror("Could not create server_client_fifo\n");
    //         _exit(1);
    //     }
    // }

    return 0;
}
