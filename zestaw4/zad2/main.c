#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

void sigusr_handler(int sig_no){
    printf("Odebrano SIGUSR1\n");
}

void check_status(){
    sigset_t set;
    sigpending(&set);
    if (sigismember(&set, SIGUSR1) == 1){
        printf("SIGUSR1 - pending\n");
    }    
}

int main(int argc, char *argv[]){
    int is_exec;
    if (argc != 2 && argc != 3){
        perror("error");
        exit(-1);
    }
    if (argc == 3){
        is_exec = 1;
    }

    if (strcmp(argv[1], "ignore") == 0){
        struct sigaction action;
        action.sa_handler = SIG_IGN;
        action.sa_flags = 0;
        sigaction(SIGUSR1, &action, NULL);
    }
    
    if (strcmp(argv[1], "handler") == 0){
        struct sigaction action;
        action.sa_handler = sigusr_handler;
        action.sa_flags = 0;
        sigaction(SIGUSR1, &action, NULL);
    }

    if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    
    printf("w procesie 'rodzicu': \n");

    raise(SIGUSR1);

    if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
        check_status();
    }

    if (is_exec == 1) {
        execl("child", "child", argv[1], NULL);
    } else {
        pid_t child_pid = fork();
        if (child_pid == 0){
            printf("w procesie 'dziecku' (fork): \n");
            if (strcmp(argv[1], "pending") != 0){
                raise(SIGUSR1);
            }

            if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
                check_status();
            }
        }   
    }
    return 0;
}