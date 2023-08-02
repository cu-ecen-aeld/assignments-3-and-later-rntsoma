#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define CLIENT_IP_SIZE 30

typedef struct node {
    pthread_t pthread_id;
    bool completed;
    struct node *next_node;
    int client_socket_fd;
    char client_ip[CLIENT_IP_SIZE];
} node;

#define FREE_RESOURCES(head, tmp) \
    while (head->next_node != NULL) { \
        if (head->next_node->completed == true) { \
            pthread_join(head->next_node->pthread_id, NULL); \
            close(head->next_node->client_socket_fd); \
            tmp = head->next_node; \
            head->next_node = head->next_node->next_node; \
            free(tmp); \
        } else { \
            head = head->next_node; \
        } \
    }\

#define INIT_NODE(new_node) \
    new_node = malloc(sizeof(node)); \
    memset(new_node, 0, sizeof(node)); \
    new_node->pthread_id = -1; \
    new_node->completed = false; \
    new_node->next_node = NULL;

#define INSERT_NODE(head, new_node) \
    while (head->next_node != NULL) { \
        head = head->next_node; \
    } \
    head->next_node = new_node; \

#define SET_HEAD_PTR(head, head_ptr) head_ptr = head;

#define PRINT_LIST(head) \
    while (head != NULL) { \
        printf("id: %lu\n", head->pthread_id); \
        head = head->next_node; \
    }


