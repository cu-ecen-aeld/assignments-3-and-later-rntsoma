#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 10
#define BUFF_SIZE 1024
#define CLIENT_IP_SIZE 30

int socket_fd;
int client_socket_fds[MAX_CONNECTIONS];
struct sockaddr_in address;

void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        printf("Caught signal, exiting\n");
        /* syslog(LOG_USER | LOG_PERROR, "Caught signal, exiting\n"); */

        for (int i=0; i<MAX_CONNECTIONS; i++) {
            if (client_socket_fds[i] != 0) {
                close(client_socket_fds[i]);
            }
        }

        /* remove("/var/tmp/aesdsocketdata"); */
        close(socket_fd);
        exit(1);
    }
}

void start() {
    int ret;

    ret = listen(socket_fd, MAX_CONNECTIONS);
    if (ret == -1) {
        printf("FAILED to listen\n");
        exit(-1);
    }

    while (1) {
        int client_addr_len;
        int *client_socket_fd = NULL;
        struct sockaddr_in client_addr;
        char client_ip[CLIENT_IP_SIZE];
        char *buffer;

        for (int i=0; i<MAX_CONNECTIONS; i++) {
            if (client_socket_fds[i] == 0) {
                client_socket_fd = &client_socket_fds[i];
            }
        }

        if (client_socket_fd != NULL) {
            memset(client_ip, 0, CLIENT_IP_SIZE * sizeof(char));
            /* memset(&client_addr, 0, sizeof(struct sockaddr_in)); */

            *client_socket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
            if (*client_socket_fd == -1) {
                printf("FAILED to accept\n");
                exit(-1);
            }
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, CLIENT_IP_SIZE * sizeof(char));
            printf("Accepted connection from %s\n", client_ip);
            /* syslog(LOG_USER | LOG_PERROR, "Accepted connection from %s\n", client_ip); */

            buffer = (char*)malloc(BUFF_SIZE * sizeof(char));
            memset(buffer, 0, BUFF_SIZE * sizeof(char));
            ret = recv(*client_socket_fd, buffer, BUFF_SIZE, 0);
            printf("Received: %s", buffer);

            FILE *fp = fopen("/var/tmp/aesdsocketdata", "a");
            fprintf(fp, "%s", buffer);
            fclose(fp);

            ssize_t bytes_read;
            int file_fd = open("/var/tmp/aesdsocketdata", O_RDONLY);
            char *fileBuffer = (char*)malloc(BUFF_SIZE * sizeof(char));
            memset(fileBuffer, 0, BUFF_SIZE * sizeof(char));
            while ((bytes_read = read(file_fd, fileBuffer, BUFF_SIZE * sizeof(char))) > 0) {
                send(*client_socket_fd, fileBuffer, bytes_read, 0);
            }

            free(fileBuffer);

            close(*client_socket_fd);
            *client_socket_fd = 0;

            printf("Closed connection from %s\n", client_ip);
            /* syslog(LOG_USER | LOG_PERROR, "Closed connection from %s\n", client_ip); */
        }
    }

    close(socket_fd);

}

int main(int argc, char **argv) {
    int ret;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    memset(&client_socket_fds, 0, MAX_CONNECTIONS * sizeof(int));

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        printf("FAILED to create socket\n");
        exit(-1);
    }

    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons(9000);
    address.sin_addr.s_addr = INADDR_ANY;
    ret = bind(socket_fd, (struct sockaddr *) &address, sizeof(address));

    if (ret == -1) {
        printf("FAILED to bind socket\n");
        exit(-1);
    }

    // Run logic
    if (argc == 1) {
        start();
    } else if (argc == 2 && strncmp(argv[1], "-d", 2) == 0) {
        printf("Daemon mode");
        ret = fork();

        if (ret == 0) {
            ret = fork();
            if (ret == 0) {
                start();
            }
        } else {
            waitpid(ret, NULL, 0);
        }
    } else {
        printf("Invalid argument\n");
        exit(-1);
    }
    return 0;
}
