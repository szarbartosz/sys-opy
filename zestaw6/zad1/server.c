#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "util.h"


int server_queue;
int distinct_id = 0;
int clients_counter = 0;
client* clients[MAX_CLIENTS] = {NULL};


void stop_handler(message* msg) {
    int client_id = atoi(msg -> text);

    int client_offset;
    for (int i = 0; i < clients_counter; i++) {
        if (clients[i] -> id == client_id) {
            client_offset = i;
            break;
        }
    }

    client* client_to_delete = clients[client_offset];

    for (int i = client_offset; i < clients_counter - 1; i++) {
        clients[i] = clients[i + 1];
    }
    clients[clients_counter - 1] = NULL;
    clients_counter--;

    free(client_to_delete);
}


void stop_server() {
    message stop_server;
    stop_server.type = STOP_SERVER;
    for (int i = 0; i < clients_counter; i++) {
        msgsnd(clients[i] -> queue_id, &stop_server, TEXT_LEN, 0);
    }

    while (clients_counter > 0) {
        message stop_client;
        msgrcv(server_queue, &stop_client, TEXT_LEN, STOP, 0);
        stop_handler(&stop_client);
    }

    msgctl(server_queue, IPC_RMID, NULL);
    exit(0);
}


void sigint_handler(int signum) {
    puts("SIGINT");
    stop_server();
}


client* get_client(int client_id) {
    for (int i = 0; i < clients_counter; i++) {
        if (clients[i] -> id == client_id) {
            return clients[i];  
        } 
    }
    return NULL;
}


void init_handler(message* msg) {
    int queue_id = atoi(msg->text);

    client* new_client = calloc(1, sizeof(client));

    new_client->id = distinct_id++;
    new_client->queue_id = queue_id;
    new_client->connected_client_id = -1;

    clients[clients_counter] = new_client;
    clients_counter++;

    message reply;
    reply.type = INIT;
    sprintf(reply.text, "%d", new_client->id);
    msgsnd(queue_id, &reply, TEXT_LEN, 0);
}


void list_handler(message* msg) {
    int client_id = atoi(msg -> text);

    client* client = get_client(client_id);

    message reply;
    reply.type = LIST;
    for (int i = 0; i < clients_counter; i++) {
        sprintf(reply.text + strlen(reply.text), "%d: %d\n", clients[i]->id,
                clients[i]->connected_client_id == -1);
    }
    msgsnd(client -> queue_id, &reply, TEXT_LEN, 0);
    puts(reply.text);
}


void connect_handler(message* msg) {
    int client_id = atoi(strtok(msg -> text, " "));
    int second_id = atoi(strtok(NULL, " "));

    client* first = get_client(client_id);
    client* second = get_client(second_id);

    first -> connected_client_id = second -> id;
    second -> connected_client_id = first -> id;

    message reply;
    reply.type = CONNECT;
    sprintf(reply.text, "%d", first -> queue_id);
    msgsnd(second -> queue_id, &reply, TEXT_LEN, 0);
    sprintf(reply.text, "%d", second -> queue_id);
    msgsnd(first -> queue_id, &reply, TEXT_LEN, 0);
}


void disconnect_handler(message* msg) {
    int client_id = atoi(msg -> text);

    client* first = get_client(client_id);
    client* second = get_client(first->connected_client_id);

    first -> connected_client_id = -1;
    second -> connected_client_id = -1;

    message reply;
    reply.type = DISCONNECT;
    msgsnd(second -> queue_id, &reply, TEXT_LEN, 0);
}


int main() {
    char* home_path = getpwuid(getuid()) -> pw_dir;
    key_t server_queue_key = ftok(home_path, SERVER_KEY_ID);
    server_queue = msgget(server_queue_key, IPC_CREAT | 0666);

    signal(SIGINT, sigint_handler);

    while (1) {
        message msg;
        msgrcv(server_queue, &msg, TEXT_LEN, -TYPES_COUNT, 0);
        printf("%ld: %s\n", msg.type, msg.text);

        switch (msg.type) {
            case INIT:
                init_handler(&msg);
                break;
            case LIST:
                list_handler(&msg);
                break;
            case CONNECT:
                connect_handler(&msg);
                break;
            case DISCONNECT:
                disconnect_handler(&msg);
                break;
            case STOP:
                stop_handler(&msg);
                break;
        }
    }
}