#define _XOPEN_SOURCE 500
#define MAX_COLUMNS_NUMBER 1000
#define MAX_LINE_LENGTH (MAX_COLUMNS_NUMBER * 5)
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>


int pair_number = 0;


typedef struct {
    int **values;
    int rows;
    int cols;
} Matrix;


struct Task{
    int pairIndex;
    int columnIndex;
};


int count_columns(char *row){
    int number = 0;
    char *element = strtok(row, " ");
    while (element != NULL){
        if (strcmp(element, "\n") != 0){
            number++;
        }
        element = strtok(NULL, " ");
    }
    return number;
}


void set_matrix_size(FILE *file, int *rows, int *cols){
    char line[MAX_LINE_LENGTH];
    *cols = 0;
    *rows = 0;

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        if (*cols == 0){
            *cols = count_columns(line);
        }
        *rows = *rows + 1;
    }

    fseek(file, 0, SEEK_SET);
}


Matrix *init_matrix(char *path){
    FILE *file = fopen(path, "r");

    int rows, cols;
    set_matrix_size(file, &rows, &cols);


    int **values = calloc(rows, sizeof(int*));

    for (int i = 0; i < rows; i++){
        values[i] = calloc(cols, sizeof(int));
    }

    int x = 0;
    int y = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        x = 0;
        char *number = strtok(line, " ");
        while (number != NULL){
            values[y][x] = atoi(number);
            x++;
            number = strtok(NULL, " ");
        }
        y++;
    }

    fclose(file);

    Matrix *matrix = calloc(1, sizeof(Matrix));
    matrix -> cols = cols;
    matrix -> rows = rows;
    matrix -> values = values;

    return matrix;
}


void print_matrix(Matrix *matrix){
    for (int i = 0; i < matrix -> rows; i++){
        for (int j= 0; j < matrix -> cols; j++){
            printf("%d ", matrix -> values[i][j]);
        }
        printf("\n");
    }
}


void delete_matrix(Matrix *matrix){
    for (int i = 0; i < matrix -> rows; i++){
        free(matrix -> values[i]);
    }
    free(matrix -> values);
}


void write_matrix_to_file(FILE *file, Matrix a){
    fseek(file, 0, SEEK_SET);
    for (int y = 0; y < a.rows; y++) {
        for (int x = 0; x < a.cols; x++) {
            if (x > 0) {
                fprintf(file, " ");
            }
            fprintf(file, "%d", a.values[y][x]);
        };
        fprintf(file, "\n");
    }
}


void generate_matrix(int rows, int cols, char* filename) {
    FILE* file = fopen(filename, "w+");
   
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (x > 0) {
                fprintf(file, " ");
            }
            fprintf(file, "%d", 0);
        };
        fprintf(file, "\n");
    }
    fclose(file);
}


void column_product(char *path1, char *path2, int columnIndex, int pairIndex){
    Matrix *matrix1 = init_matrix(path1);
    Matrix *matrix2 = init_matrix(path2);
    char *fileName = calloc(20, sizeof(char));

    sprintf(fileName, "tmp/part%d%04d", pairIndex, columnIndex);
    FILE *partFile  = fopen(fileName, "w+");

    for (int j = 0; j < matrix1 -> rows; j++){
        int result = 0;

        for (int i = 0; i < matrix1 -> cols; i++){
            result += matrix1 -> values[j][i] * matrix2 -> values[i][columnIndex];
        }

        if (j == matrix1 -> rows - 1){
            fprintf(partFile,"%d ", result);
        } else {
            fprintf(partFile, "%d \n",  result);
        }
    }
    free(matrix1);
    free(matrix2);
    fclose(partFile);
}


void one_file_product(char *path1, char *path2, int columnIndex, char *resultFile){
    Matrix *matrix1 = init_matrix(path1);
    Matrix *matrix2 = init_matrix(path2);

    FILE *file = fopen(resultFile, "r+");
    int fd = fileno(file);
    flock(fd, LOCK_EX);
    Matrix *matrix3 = init_matrix(resultFile);

    for (int j = 0; j < matrix1 -> rows; j++){
        int result = 0;
        for (int i = 0; i < matrix1 -> cols; i++){
            result += matrix1 -> values[j][i] * matrix2 -> values[i][columnIndex];
        }
        matrix3 -> values[j][columnIndex] = result;
    }

    write_matrix_to_file(file, *matrix3);
    flock(fd, LOCK_UN);
    fclose(file);
}


struct Task get_task() {
    struct Task task;
    task.columnIndex = -1;
    task.pairIndex = -1;
    for (int i = 0; i < pair_number; i++){
        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "tmp/tasks%d", i);
        FILE* tasks_file = fopen(task_filename, "r+");
        int fd = fileno(tasks_file);
        flock(fd, LOCK_EX);

        char* tasks = calloc(1000, sizeof(char));
        fseek(tasks_file, 0, SEEK_SET);
        fread(tasks, 1, 1000, tasks_file);

        char* task_first_zero = strchr(tasks, '0');
        int task_index = task_first_zero != NULL ? task_first_zero - tasks : -1;

        if (task_index >= 0) {
            char* end_of_line = strchr(tasks, '\0');
            int size = end_of_line - tasks;

            char* tasks_with_good_size = calloc(size +1, sizeof(char));
            for(int j=0; j<size; j++){
                tasks_with_good_size[j] = tasks[j];
            }
            tasks_with_good_size[task_index] = '1';
            fseek(tasks_file, 0, SEEK_SET);
            fwrite(tasks_with_good_size, 1, size, tasks_file);
            task.pairIndex = i;
            task.columnIndex = task_index;
            flock(fd, LOCK_UN);
            fclose(tasks_file);
            break;
        }
        flock(fd, LOCK_UN);
        fclose(tasks_file);
    }
    return task;
}


int process(char **filesA, char **filesB, int timeout, int mode, char **result){
    time_t start = time(NULL);
    int count = 0;
    while (1 == 1){
        
        if ((time(NULL) - start) >= timeout){
            break;
        }
        
        struct Task task = get_task();
        
        if (task.columnIndex == -1){
            break;
        }
        if (mode == 1){
            one_file_product(filesA[task.pairIndex], filesB[task.pairIndex], task.columnIndex, result[task.pairIndex]);
        }else{
            column_product(filesA[task.pairIndex], filesB[task.pairIndex], task.columnIndex, task.pairIndex);
        }
        count++;
    }
    return count;
}


char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc, char* argv[]){
        if (argc != 5){
        perror("syntax error");
    }

    int numberOfProcess = atoi(argv[2]);
    int timeout = atoi(argv[3]);
    int mode = atoi(argv[4]);

    system("rm -rf tmp");
    system("mkdir tmp");

    char **filesA = calloc(100, sizeof(char*));
    char **filesB = calloc(100, sizeof(char*));
    char **filesC = calloc(100, sizeof(char*));

    FILE *lista = fopen(argv[1], "r");
    char line[PATH_MAX * 3 + 3];
    
    int pairCount = 0;

    while (fgets(line, PATH_MAX * 3 + 3, lista) != NULL){
        filesA[pairCount] = calloc(PATH_MAX, sizeof(char));
        filesB[pairCount] = calloc(PATH_MAX, sizeof(char));
        filesC[pairCount] = calloc(PATH_MAX, sizeof(char));
        
        strcpy(filesA[pairCount], strtok(line, " "));
        strcpy(filesB[pairCount], strtok(NULL, " "));
        strcpy(filesC[pairCount], strtok(NULL, " "));
        Matrix *a = init_matrix(filesA[pairCount]);
        Matrix *b = init_matrix(filesB[pairCount]);
        if (mode == 1){
            generate_matrix(a -> rows, b -> cols, filesC[pairCount]);
        }

        char *taskFileChar = calloc(100, sizeof(char));
        sprintf(taskFileChar, "tmp/tasks%d", pairCount);
        

        FILE *taskFile = fopen(taskFileChar, "w+");
        char *tasks = calloc(b -> cols + 1, sizeof(char));
        sprintf(tasks, "%0*d", b -> cols, 0);
        fwrite(tasks, 1, b -> cols, taskFile);

        free(tasks);
        free(taskFileChar);
        fclose(taskFile);

        pairCount++;
    }
    pair_number = pairCount;

    pid_t *processes = calloc(numberOfProcess, sizeof(int));

    for (int i = 0; i < numberOfProcess; i++){
        pid_t worker = fork();
        if (worker == 0){
            return process(filesA, filesB, timeout, mode, filesC);
        } else {
            processes[i] = worker;
        }
    }

    for (int i = 0; i < numberOfProcess; i++){
        int status;
        waitpid(processes[i], &status, 0);
        printf("PID: %d      Ilosc mnozen: %d\n", processes[i], WEXITSTATUS(status));
    }
    free(processes);

    return 0;
}