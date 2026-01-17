#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 4000
#define BUFFER_SIZE 1024
// Handles SIGCHLD to clean up zombie processes
void handle_sigchld(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
// Reads a GET request from the client, finds the file, and sends its content
void handle_request(int nfd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(nfd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(nfd);
        return;
    }
    buffer[bytes_read] = '\0';

    char *method = strtok(buffer, " ");
    char *filename = strtok(NULL, " \n");
    
    if (method && filename && strcmp(method, "GET") == 0) {
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("Failed to open file");
            write(nfd, "Error: File not found\n", 22);
            close(nfd);
            return;
        }

        ssize_t bytes;
        while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
            write(nfd, buffer, bytes);
        }
        close(fd);
    } else {
        write(nfd, "Error: Invalid request\n", 23);
    }
    
    close(nfd);
}
// Runs the server, accepts clients, and forks a new process for each one
void run_service(int fd) {
    signal(SIGCHLD, handle_sigchld);

    while (1) {
        int nfd = accept_connection(fd);
        if (nfd != -1) {
            pid_t pid = fork();
            if (pid == 0) {
                close(fd);
                handle_request(nfd);
                exit(0);
            }
            close(nfd);
        }
    }
}
// Entry point: sets up the server and starts handling requests
int main(void) {
    int fd = create_service(PORT);
    if (fd == -1) {
        perror("Failed to create service");
        exit(1);
    }
    run_service(fd);
    close(fd);
    return 0;
}

