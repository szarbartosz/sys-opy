#define _POSIX_C_SOURCE 199309L
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "util.h"

int own_id;
int client_queue;
int server_queue;
int other_queue = -1;

void stop_client() {
    message msg;
    msg.type = STOP;
    sprintf(msg.text, "%d", own_id);

    msgsnd(server_queue, &msg, TEXT_LEN, 0);

    puts("Deleting queue...");
    msgctl(client_queue, IPC_RMID, NULL);
    exit(0);
}


void sigint_handler() { 
    stop_client(); 
}


void get_replies(union sigval sv) {
    (void)sv;
    message reply;
    while (msgrcv(client_queue, &reply, TEXT_LEN, -TYPES_COUNT, IPC_NOWAIT) != -1) {
        if (reply.type == CONNECT) {
            other_queue = atoi(reply.text);
        } else if (reply.type == SEND) {
            printf("MESSAGE: %s", reply.text);
        } else if (reply.type == DISCONNECT) {
            other_queue = -1;
        } else if (reply.type == STOP_SERVER) {
            stop_client();
        } else {
            puts(reply.text);
        }
    }
}


void set_timer() {
    timer_t timer;
    struct sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = get_replies;
    event.sigev_notify_attributes = NULL;
    event.sigev_value.sival_ptr = NULL;
    timer_create(CLOCK_REALTIME, &event, &timer);
    struct timespec ten_ms = {0, 10000000};
    struct itimerspec timer_value = {ten_ms, ten_ms};
    timer_settime(timer, 0, &timer_value, NULL);
}


int starts_with(char *s, char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}


int main() {
    char *home_path = getpwuid(getuid())->pw_dir;

    key_t server_queue_key = ftok(home_path, SERVER_KEY_ID);
    server_queue = msgget(server_queue_key, 0666);

    key_t client_queue_key = ftok(home_path, getpid());
    client_queue = msgget(client_queue_key, IPC_CREAT | 0666);

    signal(SIGINT, sigint_handler);

    message msg_init;
    msg_init.type = INIT;
    sprintf(msg_init.text, "%d", client_queue);
    msgsnd(server_queue, &msg_init, TEXT_LEN, 0);

    message init_ans;
    msgrcv(client_queue, &init_ans, TEXT_LEN, INIT, 0);
    own_id = atoi(init_ans.text);
    rintf("client id: %d\n", own_id);

    set_timer();
    char line[128];

    while (fgets(line, sizeof(line), stdin)) {
        message msg;
        msg.type = -1;
        int is_msg_to_client = 0;

        if (starts_with(line, "LIST")) {
            msg.type = LIST;
            sprintf(msg.text, "%d", own_id);
        }

        if (starts_with(line, "CONNECT")) {
            msg.type = CONNECT;

            (void)strtok(line, " ");
            int second_id = atoi(strtok(NULL, " "));
            sprintf(msg.text, "%d %d", own_id, second_id);
        }

        if (starts_with(line, "SEND") && other_queue != -1) {
            msg.type = SEND;

            sprintf(msg.text, "%s", strchr(line, ' ') + 1);
            is_msg_to_client = 1;
        }

        if (starts_with(line, "DISCONNECT")) {
            msg.type = DISCONNECT;
            sprintf(msg.text, "%d", own_id);
            other_queue = -1;
        }

        if (starts_with(line, "STOP")) {
            stop_client();
        }

        if (msg.type != -1) {
            int destination = is_msg_to_client ? other_queue : server_queue;
            msgsnd(destination, &msg, TEXT_LEN, 0);
        }
    }
    stop_client();
}