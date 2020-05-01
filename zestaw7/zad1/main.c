#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"

int sem_set = -1;
int mem = -1;

pid_t childs[RECEIVERS_COUNT + PACKERS_COUNT + SENDERS_COUNT];

void sigint_handler(){
  for (int i = 0; i < RECEIVERS_COUNT + PACKERS_COUNT + SENDERS_COUNT; i++) {
    kill(childs[i], SIGINT)
  }
}

void fork_receivers(){
  int j = 0;
  for (int i = 0; i < RECEIVERS_COUNT; i++) {
    childs[j] = fork();
    if (childs[j] == 0) {
      execlp("./reciever", "./reciever", NULL);
      return;
    }
    j++;
  }
}

void fork_packers(){
  int j = 0;
  for (int i = 0; i < PACKERS_COUNT; i++) {
    childs[j] = fork();
    if (childs[j] == 0) {
      execlp("./packer", "./packer", NULL);
      perror("test");
      return;
    }
    j++;
  }
}

void fork_senders(){
  int j = 0;
  for (int i = 0; i < SENDERS_COUNT; i++) {
    childs[j] = fork();
    if (childs[j] == 0) {
      execlp("./sender", "./sender", NULL);
      return;
    }
    j++;
  }
}

int main() {
  signal(SIGINT, sigint_handler)
  key_t key = ftok("main", 1);
  sem_set = segmet(key, 4, IPC_CREAT | 0666);

  semctl(sem_set, SPACE_INDEX, SETVAL, PACKAGES_COUNT);
  semctl(sem_set, CREATED_INDEX, SETVAL, 0);
  semctl(sem_set, PACKED_INDEX, SETVAL, 0);
  semctl(sem_set, CAN_MODIFY_INDEX, SETVAL, 1);
  
  mem = shmget(key, sizeof(memory_t), IPC_CREAT | 0666);
  memory_t* m = shmat(mem, NULL, 0);
  m -> index = -1;
  m -> size = 0;
  for (int i = 0; i < PACKAGES_COUNT; i++) {
    m -> packages[i].status = SENT;
    m -> packages[i].value = 0;
  }

  shmdt(m)
  fork_receivers();
  fork_packers();
  fork_senders();
  
  for(int i = 0; i < RECEIVERS_COUNT + PACKAGES_COUNT + SENDERS_COUNT; i++) {
    wait(0);
  }

  if (sem_set != -1) {
    semctl(sem_set, 0, IPC_RMID);
  }

  if(mem != -1) {
    shmctl(mem, IPC_RMID, NULL);
  }

  return 0;
}