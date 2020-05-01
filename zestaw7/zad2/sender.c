#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "util.h"

int main() {
  sem_t* space = sem_open("/space", O_RDWR, 0666);
  sem_t* created = sem_open("/created", O_RDWR, 0666);
  sem_t* packed = sem_open("/packed", O_RDWR, 0666);
  sem_t* can_modify = sem_open("/can_modify", O_RDWR, 0666);
  int mem = shm_open("/memory", O_RDWR, 0666);

  while (1) {
    sem_wait(packed);
    sem_wait(can_modify);

    memory_t* m = mmap(NULL, sizeof(memory_t), PROT_WRITE, MAP_SHARED, mem, 0);

    m -> packages[m -> index].status = SENT;
    m -> packages[m -> index].value /= 2;
    m -> packages[m -> index].value *= 3;
    int num = m -> packages[m -> index].value;
    m -> index = (m -> index + 1) % PACKAGES_COUNT;
    m -> size--;

    int created_no;
    sem_getvalue(created, &created_no);
    int packed_no;
    sem_getvalue(packed, &packed_no);

    printf("(%d %lu) wyslalem zamowienie o wielkosci %d.", getpid(), time(NULL), num);
    printf("liczba paczek do przygotowania: %d.", created_no);
    printf("liczba paczek do wyslania: %d\n", packed_no);

    sem_post(space);
    sem_post(can_modify);

    munmap(m, sizeof(memory_t));
    sleep(1);
  }
}