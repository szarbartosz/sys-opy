#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

int own_id;
mqd_t client_queue;
mqd_t server_queue;
mqd_t other_queue = -1;
char filename[FILENAME_LEN + 1];


void stop_client() {
    char msg[TEXT_LEN + 1] = {0};
    sprintf(msg, "%d", own_id);

    send_message(server_queue, STOP, msg);

    mq_unlink(filename);
    exit(0);
}


void sigint_handler() { 
  stop_client(); 
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


void register_notification();


void notification_handler(union sigval sv) {
    register_notification();
    char *text;
    char type;
    while ((text = read_message(client_queue, &type)) != NULL) {
        switch (type) {
            case CONNECT:
                other_queue = mq_open(text, O_RDWR, 0666, NULL);
                break;
            case SEND:
                printf("MESSAGE: %s", text);
                break;
            case DISCONNECT:
                if (other_queue) {
                    mq_close(other_queue);
                }
                other_queue = -1;
                break;
            case STOP_SERVER:
                stop_client();
                break;
            default:
                puts(text);
        }
    }
}


void register_notification() {
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = notification_handler;
    event.sigev_notify_attributes = NULL;
    event.sigev_value.sival_ptr = NULL;

    mq_notify(client_queue, &event);
}


void set_nonblocking() {
    struct mq_attr attr;
    mq_getattr(client_queue, &attr);
    attr.mq_flags = O_NONBLOCK;
    mq_setattr(client_queue, &attr, NULL);
}

int starts_with(char *s, char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}


int main() {
    sprintf(filename, "/%d", getpid());
    client_queue = mq_open(filename, O_RDWR | O_CREAT, 0666, NULL);

    server_queue = mq_open("/server", O_RDWR, 0666, NULL);

    signal(SIGINT, sigint_handler);

    send_message(server_queue, INIT, filename);
    char *encoded_id = read_message(client_queue, NULL);
    own_id = atoi(encoded_id);
    free(encoded_id);
    printf("own_id: %d\ncommand: ", own_id);

    set_nonblocking();
    register_notification();

    char line[128];
    while (fgets(line, sizeof(line), stdin)) {
        char text[TEXT_LEN + 1] = {0};
        int type = -1;
        int is_msg_to_client = 0;

        if (starts_with(line, "LIST")) {
            type = LIST;
            sprintf(text, "%d", own_id);
        }

        if (starts_with(line, "CONNECT")) {
            type = CONNECT;

            (void)strtok(line, " ");
            int second_id = atoi(strtok(NULL, " "));
            sprintf(text, "%d %d", own_id, second_id);
        }

        if (starts_with(line, "SEND") && other_queue != -1) {
            type = SEND;

            sprintf(text, "%s", strchr(line, ' ') + 1);
            is_msg_to_client = 1;
        }

        if (starts_with(line, "DISCONNECT")) {
            type = DISCONNECT;
            sprintf(text, "%d", own_id);
            other_queue = -1;
        }

        if (starts_with(line, "STOP")) {
            stop_client();
        }

        if (type != -1) {
            mqd_t destination = is_msg_to_client ? other_queue : server_queue;
            send_message(destination, type, text);

            sleep(1);
        }
        printf("command: ");
    }
    stop_client();
}