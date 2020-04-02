#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int paused = 0;

void sigtstp_handler(int sig){
    if(!paused){
        printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu.\n");
        paused = 1;
    } else{
        paused = 0;
    }
}

void sigint_handler(int sig_no){
    printf("\nOdebrano sygnał SIGINT.\n");
    exit(0);
}


int main(){
    struct sigaction action;
    action.sa_handler = sigtstp_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTSTP, &action, NULL);

    signal(SIGINT, sigint_handler);
    while(1){
        if(!paused){
            system("ls -l");
            sleep(1);
        }
    }
}