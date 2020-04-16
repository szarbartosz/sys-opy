#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("error");
        exit(EXIT_FAILURE);
    }

    char *command = calloc(strlen(argv[1]) + 10, sizeof(char));
    sprintf(command, "sort %s", argv[1]);

    FILE* output = popen(command, "w");
    free(command);
    
    char buffer[997];

    while (fgets(buffer, sizeof(buffer) - 1, output) != NULL){
        printf("%s\n", buffer);
    }

    fclose(output);
}