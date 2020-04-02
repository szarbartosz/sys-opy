#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void check_status(){
    sigset_t set;
    sigpending(&set);
    if (sigismember(&set, SIGUSR1) == 1){
        printf("SIGUSR1 - pending\n");
    }    
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("error");
        exit(-1);
    }

    printf("w procesie 'dziecku' (exec): \n");
    if (strcmp(argv[1], "pending") != 0){
        raise(SIGUSR1);
    }

    if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){
        check_status();
    }
}