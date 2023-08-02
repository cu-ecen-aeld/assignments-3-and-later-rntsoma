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

#include "list.h"

#define MAX_CONNECTIONS 10
#define BUFF_SIZE 1024

int socket_fd;
int client_socket_fds[MAX_CONNECTIONS];
struct sockaddr_in address;
node *head;

void close_connections() {
    //TODO
    /* for (int i=0; i<MAX_CONNECTIONS; i++) { */
    /*     if (client_socket_fds[i] != 0) { */
    /*         close(client_socket_fds[i]); */
    /*     } */
    /* } */
}

void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        printf("Caught signal, exiting\n");
        /* syslog(LOG_USER | LOG_PERROR, "Caught signal, exiting\n"); */

        close_connections();

        remove("/var/tmp/aesdsocketdata");
        close(socket_fd);
        exit(-1);
    }
}

void *spawn(void *arg) {
    int ret;
    char *buffer;
    char *ptr = NULL;
    FILE *fp = fopen("/var/tmp/aesdsocketdata", "a");
    node *info = (node *)arg;

    while (1) {
        buffer = (char*)malloc(BUFF_SIZE * sizeof(char));
        memset(buffer, 0, BUFF_SIZE * sizeof(char));
        ret = recv(info->client_socket_fd, buffer, BUFF_SIZE - 1, 0);

        if (ret == 0 || ret == -1) {
            fclose(fp);
            free(buffer);
            break;
        }

        fprintf(fp, "%s", buffer);
        ptr = strchr(buffer, '\n');

        if (ptr != NULL) {
            free(buffer);
            fclose(fp);
            break;
        }
        free(buffer);
    }

    ssize_t bytes_read;
    int file_fd = open("/var/tmp/aesdsocketdata", O_RDONLY);
    char *fileBuffer = (char*)malloc(BUFF_SIZE * sizeof(char));
    memset(fileBuffer, 0, BUFF_SIZE * sizeof(char));
    while ((bytes_read = read(file_fd, fileBuffer, BUFF_SIZE * sizeof(char))) > 0) {
        send(info->client_socket_fd, fileBuffer, bytes_read, 0);
    }

    free(fileBuffer);

    close(info->client_socket_fd);
    info->completed = true;

    printf("Closed connection from %s\n", info->client_ip);
    /* syslog(LOG_USER | LOG_PERROR, "Closed connection from %s\n", info->client_ip); */
}

void start() {
    int ret;

    ret = listen(socket_fd, MAX_CONNECTIONS);
    if (ret == -1) {
        printf("FAILED to listen\n");
        goto CLOSE_SOCKET;
    }

    while (1) {
        int client_addr_len;
        int client_socket_fd;
        struct sockaddr_in client_addr;
        char client_ip[CLIENT_IP_SIZE];
        pthread_t thread_id;
        node *new_node, *head_ptr, *tmp;

        memset(&client_addr_len, 0, sizeof(int));
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        memset(client_ip, 0, CLIENT_IP_SIZE * sizeof(char));

        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket_fd == -1) {
            printf("FAILED to accept\n");
            goto CLOSE_SOCKET;
        }
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, CLIENT_IP_SIZE * sizeof(char));
        printf("Accepted connection from %s\n", client_ip);
        /* syslog(LOG_USER | LOG_PERROR, "Accepted connection from %s\n", client_ip); */

        INIT_NODE(new_node);
        SET_HEAD_PTR(head, head_ptr);
        new_node->client_socket_fd = client_socket_fd;
        memcpy(new_node->client_ip, client_ip, CLIENT_IP_SIZE);

        ret = pthread_create(&thread_id, NULL, spawn, new_node);
        new_node->pthread_id = thread_id;

        INSERT_NODE(head_ptr, new_node);
        SET_HEAD_PTR(head, head_ptr);

        FREE_RESOURCES(head_ptr, tmp);
    }

    close(socket_fd);
    return;

CLOSE_SOCKET:
    close_connections();
    close(socket_fd);
    exit(-1);
}

int main(int argc, char **argv) {
    int ret;
    int optVal = 1;

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

    ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    if (ret == -1) {
        printf("FAILED to set option\n");
        goto CLOSE_SOCKET;
    }

    ret = bind(socket_fd, (struct sockaddr *) &address, sizeof(address));
    if (ret == -1) {
        printf("FAILED to bind socket\n");
        goto CLOSE_SOCKET;
    }

    INIT_NODE(head);

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
        goto CLOSE_SOCKET;
    }
    return 0;

CLOSE_SOCKET:
    close(socket_fd);
    exit(-1);
}
