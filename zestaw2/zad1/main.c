#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


//concatenate two strings into one
char *concat(const char *str1, const char *str2)
{
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}


//generate random word of length "length"
char *generate_word(int length){
    char *word = calloc(length, sizeof(char));
    for (int i = 0; i < length; i++){
        word[i] = 65 + rand() % 26;
    }
    return word;
}


//create file of name "file_name" and fill it with "number of records" records of randomly generated words
void generate_records(char *file_name, int number_of_records, int length_of_word){
    char *create_file = concat("touch ", file_name);
    system(create_file);

    for (int i = 0; i < number_of_records; i++){
        char *word = generate_word(length_of_word);
        char *command = concat("echo ", concat(word, concat(" >> ",file_name)));
        system(command);
    }
}


//compare two strings in lexigraphical order
// if str1 > str2 -> return -1
// if str2 > str1 -> return 1
// if they are equal -> return 0
int compare_strings(char *str1, char *str2){
    int length = strlen(str1);
    for (int i = 0; i < length; i++){
        if (str1[i] > str2[i]){
            return -1;
        }
        if (str1[i] < str2[i]){
            return 1;
        }
    }
    return 0;
}


//********************************************SYSTEM FUNCTIONS********************************************


//SORTING


//return 'index'-th record from file
//fd - file descriptor
char *get_record_sys(int fd, int index, int length_of_record){
    char *record = calloc(length_of_record, sizeof(char));
    lseek(fd, (length_of_record + 1) * index, 0);
    read(fd, record, length_of_record);
    return record;
}


//save record on 'index'-th position in file
void save_record_sys(int fd, char *record, int index, int length_of_record){
    lseek(fd, (length_of_record + 1) * index, 0);
    write(fd, record, length_of_record);
}


//swap records of indexes "index1" and "index2" in file of file descriptor "fd"
void swap_records_sys(int fd, int index1, int index2, int length_of_record){
    char *tmp = get_record_sys(fd, index1, length_of_record);
    save_record_sys(fd, get_record_sys(fd, index2, length_of_record), index1, length_of_record);
    save_record_sys(fd, tmp, index2, length_of_record);
}



//partition in place
int partition_sys(int fd, int low, int high, int length_of_record){
    char *pivot = get_record_sys(fd, high, length_of_record);
    int i = low - 1;
    for (int j = low; j < high; j++){
        char *record = get_record_sys(fd, j, length_of_record);
        if (compare_strings(record, pivot) >= 0){
            i = i + 1;
            swap_records_sys(fd, i, j, length_of_record);
        }
    }
    swap_records_sys(fd, i + 1, high, length_of_record);
    return i + 1;
}


//quicksort with its recursive calls
void quick_sort_sys(int fd, int low, int high, int length_of_record){
    if (low < high){
        int q = partition_sys(fd, low, high, length_of_record);
        quick_sort_sys(fd, low, q - 1, length_of_record);
        quick_sort_sys(fd, q + 1, high, length_of_record);
    }
}


//warp 
void sort_sys(char *file_name, int number_of_records, int length_of_record){
    int fd = open(file_name, O_RDWR);
    if (fd == -1){
        perror("error");
        // fprintf(stderr, "error: %s\n", strerror(errno));
        // exit(-1);
    }

    quick_sort_sys(fd, 0, number_of_records - 1, length_of_record);
    close(fd);
}



//COPYING


void copy_sys(char *file_name1, char *file_name2, int number_of_records, int length_of_record){
    int fd1 = open(file_name1, O_RDONLY);
    if (fd1 == -1){
        perror("error");
    }

    int fd2 = open(file_name2, O_WRONLY);
    if (fd2 == -1){
        system("touch copy.txt");
        fd2 = open("copy.txt", O_WRONLY);
    }

    char buffer[length_of_record + 1];
    for (int i = 0; i < number_of_records; i++){
        read(fd1, buffer, length_of_record + 1);
        write(fd2, buffer, length_of_record + 1);
    }

    close(fd1);
    close(fd2);
}



//********************************************LIBRARY FUNCTIONS********************************************


//SORTING


char *get_record_lib(FILE *file, int index, int length_of_record){
    char *record = calloc(length_of_record, sizeof(char));
    fseek(file, (length_of_record + 1) * index, 0);
    fread(record, sizeof(char), length_of_record, file);
    return record;
}

void save_record_lib(FILE *file, char *record, int index, int length_of_record){
    fseek(file, (length_of_record + 1) * index, 0);
    fwrite(record, sizeof(char), length_of_record, file);
}

void swap_records_lib(FILE *file, int index1, int index2, int length_of_record){
    char *tmp = get_record_lib(file, index1, length_of_record);
    save_record_lib(file, get_record_lib(file, index2, length_of_record), index1, length_of_record);
    save_record_lib(file, tmp, index2, length_of_record);
}

int partition_lib(FILE *file, int low, int high, int length_of_record){
    char *pivot = get_record_lib(file, high, length_of_record);
    int i = low - 1;
    for (int j = low; j < high; j++){
        char *record = get_record_lib(file, j, length_of_record);
        if (compare_strings(record, pivot) >= 0){
            i = i + 1;
            swap_records_lib(file, i, j, length_of_record);
        }
    }
    swap_records_lib(file, i + 1, high, length_of_record);
    return i + 1;
}

void quick_sort_lib(FILE *file, int low, int high, int length_of_record){
    if (low < high){
        int q = partition_lib(file, low, high, length_of_record);
        quick_sort_lib(file, low, q - 1, length_of_record);
        quick_sort_lib(file, q + 1, high, length_of_record);
    }
}

void sort_lib(char *file_name, int numberOfRecords, int length_of_record){
    FILE *file = fopen(file_name, "r+");
    if (file == NULL){
        perror("error");
    }

    quick_sort_lib(file, 0, numberOfRecords - 1, length_of_record);
    fclose(file);
}


//COPYING


void copy_lib(char *file_name1, char *file_name2, int number_of_records, int length_of_record){
    FILE *file1 = fopen(file_name1, "r");
    if (file1 == NULL){
        perror("error");
    }

    FILE *file2 = fopen(file_name2, "w");
    if (file2 == NULL){
        system("touch copy.txt");
        file2 = fopen("copy.txt", "w");
    }

    char buffer[length_of_record + 1];
    for (int i = 0; i < number_of_records; i++){
        fread(buffer, sizeof(char), length_of_record + 1, file1);
        fwrite(buffer, sizeof(char), length_of_record + 1, file2);
    }

    fclose(file1);
    fclose(file2);
}

void red_command(int argc, char *argv[]){
    if (strcmp(argv[1], "generate") == 0){
        char *file_name = argv[2];
        int number_of_records = atoi(argv[3]);
        int length_of_record = atoi(argv[4]);
        generate_records(file_name, number_of_records, length_of_record);
        return;
    }

    if (strcmp(argv[1], "sort") == 0){
        char *file_name = argv[2];
        int number_of_records = atoi(argv[3]);
        int length_of_record = atoi(argv[4]);
        if (strcmp(argv[5], "sys") == 0){
            sort_sys(file_name, number_of_records, length_of_record);
            return;
        }
        if (strcmp(argv[5], "lib") == 0){
            sort_lib(file_name, number_of_records, length_of_record);
            return;
        }
    }

    if (strcmp(argv[1], "copy") == 0){
        char *file_name1 = argv[2];
        char *file_name2 = argv[3];
        int number_of_records = atoi(argv[4]);
        int length_of_record = atoi(argv[5]);
        if (strcmp(argv[6], "sys") == 0){
            copy_sys(file_name1, file_name2, number_of_records, length_of_record);
            return;
        }

        if (strcmp(argv[6], "lib") == 0){
            copy_lib(file_name1, file_name2, number_of_records, length_of_record);
            return;
        }
    }
}



int main(int argc, char *argv[]){
    read_command(argc, argv);
}
