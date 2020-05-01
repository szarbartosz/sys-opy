#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"

pid_t childs[CREATORS_COUNT + PACKERS_COUNT + SENDERS_COUNT];

void sigint_handler() {
  for (int i = 0; i < RECEIVERS_COUNT + PACKERS_COUNT + SENDERS_COUNT; i++) {
    kill(childs[i], SIGTERM);
  }
}

void fork_receivers(){
  int j = 0;
  for (int i = 0; i < RECEIVERS_COUNT; i++) {
    childs[j] = fork();
    if (childs[j] == 0) {
      execlp("./creator", "./creator", NULL);
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
  signal(SIGINT, sigint_handler);

  sem_t *space = sem_open("/space", O_CREAT | O_RDWR, 0666, PACKAGES_COUNT);
  sem_t *created = sem_open("/created", O_CREAT | O_RDWR, 0666, 0);
  sem_t *packed = sem_open("/packed", O_CREAT | O_RDWR, 0666, 0);
  sem_t *can_modify = sem_open("/can_modify", O_CREAT | O_RDWR, 0666, 1);
  int mem = shm_open("/memory", O_CREAT | O_RDWR, 0666);
  ftruncate(mem, sizeof(memory_t));

  memory_t *m = mmap(NULL, sizeof(memory_t), PROT_WRITE, MAP_SHARED, mem, 0);
  m -> index = -1;
  m -> size = 0;
  for (int i = 0; i < PACKAGES_COUNT; i++) {
      m -> packages[i].status = SENT;
      m -> packages[i].value = 0;
  }

  fork_receivers();
  fork_packers();
  fork_senders();
  munmap(m, sizeof(memory_t));

  for (int i = 0; i < RECEIVERS_COUNT + PACKERS_COUNT + SENDERS_COUNT; i++) {
    wait(0);
  }

  sem_close(space);
  sem_close(created);
  sem_close(packed);
  sem_close(can_modify);
  sem_unlink("/space");
  sem_unlink("/created");
  sem_unlink("/packed");
  sem_unlink("/can_modify");
  shm_unlink("/memory");

  return 0;
}