#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

mqd_t server_queue;
int distinct_id = 0;
int clients_counter = 0;
client* clients[MAX_CLIENTS] = {NULL};


void stop_server() {
    char text[TEXT_LEN + 1] = {0};
    for (int i = 0; i < clients_counter; i++) {
        send_message(clients[i] -> queue_id, STOP_SERVER, text);
    }

    while (clients_counter > 0) {
        char type;
        char* reply = read_message(server_queue, &type);
        puts(reply);

        if (type == STOP) {
            stop_handler(reply);
        }

        free(reply);
    }
    
    mq_close(server_queue);
    mq_unlink("/server");
    exit(0);
}

void sigint_handler(int signum) {
    (void)signum;
    stop_server();
}


char* read_message(mqd_t src, char* type) {
    char from_queue[TEXT_LEN + 2] = {0};

    int success = mq_receive(src, from_queue, TEXT_LEN + 1, NULL);
    if (success == -1) return NULL;

    if (type) {
        *type = from_queue[0];
    }
    char* retval = calloc(TEXT_LEN + 1, sizeof(char));
    sprintf(retval, "%s", from_queue + 1);

    return retval;
}


void send_message(mqd_t dest, char type, char* message) {
    int length = strlen(message);
    char* buffer = calloc(2 + length, sizeof(char));
    buffer[0] = type;
    sprintf(buffer + 1, "%s", message);
    mq_send(dest, buffer, length + 1, TYPES_COUNT - type + 1);
}


client* get_client(int client_id) {
    for (int i = 0; i < clients_counter; i++) {
        if (clients[i] -> id == client_id) {
          return clients[i];
    }
    return NULL;
}


void init_handler(char* text) {
    mqd_t queue_id = mq_open(text, O_RDWR, 0666, NULL);

    client* new_client = calloc(1, sizeof(client));
    new_client -> queue_filename = calloc(FILENAME_LEN + 1, sizeof(client));

    new_client -> id = distinct_id++;
    new_client -> queue_id = queue_id;
    sprintf(new_client -> queue_filename, "%s", text);
    new_client -> connected_client_id = -1;

    clients[clients_counter++] = new_client;

    char reply[TEXT_LEN + 1];
    sprintf(reply, "%d", new_client -> id);

    send_message(new_client -> queue_id, INIT, reply);
}


void list_handler(char* text) {
    int client_id = atoi(text);

    client* client = get_client(client_id);

    char reply[TEXT_LEN + 1] = {0};
    for (int i = 0; i < clients_counter; i++) {
        sprintf(reply + strlen(reply), "%d: %d\n", clients[i] -> id,
                clients[i] -> connected_client_id == -1);
    }

    send_message(client -> queue_id, LIST, reply);
}

void connect_handler(char* text) {
    int client_id = atoi(strtok(text, " "));
    int second_id = atoi(strtok(NULL, " "));

    client* first = get_client(client_id);
    client* second = get_client(second_id);

    first -> connected_client_id = second -> id;
    second -> connected_client_id = first -> id;

    char reply[TEXT_LEN + 1] = {0};
    sprintf(reply, "%s", first -> queue_filename);
    send_message(second -> queue_id, CONNECT, reply);
    sprintf(reply, "%s", second -> queue_filename);
    send_message(first -> queue_id, CONNECT, reply);
}


void disconnect_handler(char* text) {
    int client_id = atoi(text);

    client* first = get_client(client_id);
    client* second = get_client(first -> connected_client_id);

    first -> connected_client_id = -1;
    second -> connected_client_id = -1;

    char reply[TEXT_LEN + 1] = {0};
    send_message(second -> queue_id, DISCONNECT, reply);
}


void stop_handler(char* text) {
    int client_id = atoi(text);

    int client_offset;
    for (int i = 0; i < clients_counter; i++) {
        if (clients[i] -> id == client_id) {
            client_offset = i;
            break;
        }
    }

    client* client_to_be_deleted = clients[client_offset];

    for (int i = client_offset; i < clients_counter - 1; i++) {
        clients[i] = clients[i + 1];
    }
    clients[clients_counter - 1] = NULL;
    clients_counter--;

    mq_close(client_to_be_deleted -> queue_id);
    free(client_to_be_deleted -> queue_filename);
    free(client_to_be_deleted);
}


int main() {
    server_queue = mq_open("/server", O_RDWR | O_CREAT, 0666, NULL);

    signal(SIGINT, sigint_handler);

    while (1) {
        char type;
        char* text = read_message(server_queue, &type);
        printf("%d: %s\n", type, text);

        switch (type) {
            case INIT:
                init_handler(text);
                break;
            case LIST:
                list_handler(text);
                break;
            case CONNECT:
                connect_handler(text);
                break;
            case DISCONNECT:
                disconnect_handler(text);
                break;
            case STOP:
                stop_handler(text);
                break;
        }
        free(text);
    }

    stop_server();
}