#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("error");
        exit(EXIT_FAILURE);
    }

    FILE *pipe = fopen(argv[1], "r");
    FILE *file = fopen(argv[2], "w+");
    int N = atoi(argv[3]);

    char *buffer = calloc(N + 1, sizeof(char));
    while (fread(buffer, 1, N, pipe) > 0) {
        fwrite(buffer, 1, N, file);
    }

    free(buffer);
    fclose(file);
    fclose(pipe);
}