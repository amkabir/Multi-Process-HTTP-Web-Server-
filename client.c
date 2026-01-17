#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define PORT 4000
#define MIN_ARGS 2
#define MAX_ARGS 2
#define SERVER_ARG_IDX 1
#define USAGE_STRING "usage: %s <server address>\n"
// Checks if the right number of arguments are given
void validate_arguments(int argc, char *argv[]) {   
    if (argc < MIN_ARGS || argc > MAX_ARGS) {   
        fprintf(stderr, USAGE_STRING, argv[0]);
        exit(EXIT_FAILURE);
    }
}
// Sends user input to the server and prints the response
void send_request(int fd) {
    char *line = NULL;
    size_t size;
    ssize_t num;
    char buffer[100];

    while ((num = getline(&line, &size, stdin)) >= 0) {
        write(fd, line, num);
        ssize_t bytes_received = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
        }
    }
    free(line);
}
// Connects to the given server
int connect_to_server(struct hostent *host_entry) {
    int fd;
    struct sockaddr_in their_addr;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);

    if (connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        close(fd);
        perror(0);
        return -1;
    }
    return fd;
}
// Resolves the hostname to an IP address
struct hostent *gethost(char *hostname) {
    struct hostent *he;
    if ((he = gethostbyname(hostname)) == NULL) {
        perror("gethostbyname failed");
	return NULL;
    }
    return he;
}
// Main function: validates args, connects to server, sends requests
int main(int argc, char *argv[]) {
    validate_arguments(argc, argv);
    struct hostent *host_entry = gethost(argv[SERVER_ARG_IDX]);

    if (host_entry) {
        int fd = connect_to_server(host_entry);
        if (fd != -1) {
            send_request(fd);
            close(fd);
        }
    }
    return 0;
}

