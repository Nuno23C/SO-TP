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

char* itoa(int val, int base) {

	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}


void receiveRequest() {
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

    char string[SIZE];
    int bytesRead = 0;
    if ((bytesRead = read(client_server, string, SIZE)) == -1) {
        perror("Error reading from server");
    }
    printf("Recebi a string: %s\n", string);
    printf("bytesRead = %d\n", bytesRead);

    while (bytesRead > 0) {
        write(1, string, bytesRead);
    }

    close(client_server);

    int server_client = open("server_client_fifo", O_WRONLY, 0666);
    if(server_client == -1){
		perror("Could not open the fifo\n");
		_exit(1);
	}
}

int main(int argc, char **argv){
    if (mkfifo("server_client_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Could not create server_client_fifo\n");
            _exit(1);
        }
    }

    receiveRequest();

    return 0;
}
