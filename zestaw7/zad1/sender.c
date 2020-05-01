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
  int sem_set = semget(key, 4, 0);
  int mem = shmget(key, sizeof(memory_t), 0);

  struct sembuf lock_memory = {CAN_MODIFY_INDEX, -1, 0};
  struct sembuf decrement_packed = {PACKED_INDEX, -1, 0};
  struct sembuf ops_start[2] = {lock_memory, decrement_packed};

  struct sembuf unlock_memory = {CAN_MODIFY_INDEX, 1, 0};
  struct sembuf increment_space = {SPACE_INDEX, 1, 0};
  struct sembuf ops_end[2] = {unlock_memory, increment_space};

  while (1) {
    semop(sem_set, ops_start, 2);
    memory_t *m = shmat(mem, NULL, 0);

    m -> packages[m -> index].status = SENT;
    m -> packages[m -> index].value /= 2;
    m -> packages[m -> index].value *= 3;
    int num = m -> packages[m -> index].value;
    m -> index = (m -> index + 1) % PACKAGES_COUNT;
    m -> size--;

    int created_no = semctl(sem_set, CREATED_INDEX, GETVAL);
    int packed_no = semctl(sem_set, PACKED_INDEX, GETVAL);

    printf("(%d %lu) wyslalem zamowienie o wielkosci: %d.", getpid(), time(NULL), num);
    printf("liczba paczek do przygotowania: %d.", created_no);
    printf("liczba paczek do wyslania: %d\n", packed_no);

    semop(sem_set, ops_end, 2);
    shmdt(m);
    sleep(1);
  }
}