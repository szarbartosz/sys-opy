#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "util.h"

int main() {
  key_t key = ftok("main", 1);
  int sem_set = segmet(key, 4, 0);
  int mem = shmget(key, sizeof(memory_t), 0);

  struct sembuf lock_memory = {CAN_MODIFY_INDEX, -1, 0};
  struct sembuf decrement_space = {SPACE_INDEX, -1, 0};
  struct sembuf ops_start[2] = {lock_memory, decrement_space};

  struct sembuf unlock_memory = {CAN_MODIFY_INDEX, 1, 0};
  struct sembuf increment_created = {CREATED_INDEX, 1, 0};
  struct sembuf ops_end[2] = {unlock_memory, increment_created};

  while (1) {
    semop(sem_set, ops_start, 2);
    int num = rand() % MAX_CREATED_COUNT;
    memory_t* m = shmat(mem, NULL, 0);

    int i;
    if (m -> index == -1) {
      m -> index = 0;
      i = 0;
    } else {
      i = (m -> index + m -> size) % PACKAGES_COUNT;
    }

    m -> packages[i].value = num;
    m -> packages[i].status = CREATED;
    m -> size++;

    int created_no = semctl(sem_set, CREATED_INDEX, GETVAL);
    int packed_no = semctl(sem_set, PACKED_INDEX, GETVAL);

    printf("(%d %lu) dostalem liczbe: %d.", getpid(), time(NULL), num);
    printf("liczba paczek do przygotowania: %d.", created_no);
    printf("liczba paczek do wyslania: %d.", packed_no);

    semop(sem_set, ops_end, 2);
    shmdt(m);
    sleep(1);
  }
}