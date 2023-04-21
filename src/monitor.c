#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../includes/status.h"
#include "../includes/program.h"

#define SIZE 1024

int main(int argc, char **argv){
    int client_server = open("client_server_fifo", O_RDONLY, 0666);
    if (client_server == -1){
		perror("Could not open the fifo client_server\n");
		_exit(1);
	}

    Status status;
    int bytesRead;

    if ((bytesRead = read(client_server, &status, sizeof(Status))) == -1) {
        perror("Error reading from tracer\n");
        _exit(1);
    }

    printf("process_pid: %d\n", status.process_pid);
    printf("timestampI: %ld\n", status.timestampI);
    printf("program_name: %s\n", status.program_name);


    close(client_server);

    return 0;
}
