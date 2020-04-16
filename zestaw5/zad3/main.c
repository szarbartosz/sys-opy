#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

void run_child(char* path, int producer) {
    if (fork() == 0) {
        if (producer) {
            execlp("./producer", "./producer", "pipe", path, "10", NULL);
        } else {
            execlp("./consumer", "./consumer", "pipe", path, "10", NULL);
        }
    }
}

int main() {
    mkfifo("pipe", S_IRUSR | S_IWUSR);

    run_child("6.txt", 1);
    run_child("9.txt", 1);
    run_child("J.txt", 1);
    run_child("P.txt", 1);
    run_child("G.txt", 1);
    run_child("out.txt", 0);

    for (int i = 0; i < 6; i++) {
        wait(NULL);
    }
}