#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>


char *concat(const char *str1, const char *str2)
{
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}


void print_file_info(char *path, struct stat file_stat){
    char *file_type;

    if (S_ISREG(file_stat.st_mode)){
        file_type = "file";
    } else if (S_ISDIR(file_stat.st_mode)){
        file_type = "dir";
    } else if (S_ISCHR(file_stat.st_mode)){
        file_type = "char dev";
    } else if (S_ISBLK(file_stat.st_mode)){
        file_type = "block dev";
    } else if (S_ISFIFO(file_stat.st_mode)){
        file_type = "fifo";
    } else if (S_ISLNK(file_stat.st_mode)){
        file_type = "slink";
    } else if (S_ISSOCK(file_stat.st_mode)){
        file_type = "sock";
    }

    time_t tm = file_stat.st_mtime;
    struct tm ltm;
    localtime_r(&tm, &ltm);
    char modtime[80];
    strftime(modtime, sizeof(modtime), "%c", &ltm);

    time_t ta = file_stat.st_atime;
    struct tm lta;
    localtime_r(&ta, &lta);
    char accestime[80];
    strftime(accestime, sizeof(accestime), "%c", &lta);
    
    printf("path: %s | links: %ld | file type: %s | file size: %ld | mod time: %s | acces time: %s\n",
        path, file_stat.st_nlink, file_type, file_stat.st_size, modtime, accestime);
}


void max_depth(char *path, int depth){
    if (depth == 0){
        return;
    }

    if (path == NULL){
        return;
    }

    DIR *dir = opendir(path);

    if (dir == NULL){
        perror("error");
        exit(-1);
    }

    struct dirent *file;

    while ((file = readdir(dir)) != NULL){
        char *new_path;
        new_path = concat("", concat(path, concat("/", file -> d_name)));
        struct stat file_stat;
        lstat(new_path, &file_stat);
        
        if (S_ISDIR(file_stat.st_mode)){
            if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0){
                max_depth(new_path, depth - 1);
            }
        }
        if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0)
            print_file_info(new_path, file_stat);
    }
    closedir(dir);
}


void mtime(char *path, char sign, int depth, time_t date){
    if (depth == 0){
        return;
    }

    if (path == NULL){
        return;
    }

    DIR *dir = opendir(path);

    if (dir == NULL){
        perror("error");
        exit(-1);
    }

    struct dirent *file;

    while ((file = readdir(dir)) != NULL){
        char *new_path;
        new_path = concat("", concat(path, concat("/", file -> d_name)));
        struct stat file_stat;
        lstat(new_path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)){
            if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0){
                mtime(new_path, sign, depth - 1, date);
            }
        }

        time_t modtime = file_stat.st_mtime;
        int diff = difftime(date, modtime);

        if ((sign == '-' && diff <= 0) || (sign == '+' && diff >= 0)){
            if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0)
            print_file_info(new_path, file_stat);
        } 
    }
    closedir(dir);
}


void atime(char *path, char sign, int depth, time_t date){
    if (depth == 0){
        return;
    }

    if (path == NULL){
        return;
    }

    DIR *dir = opendir(path);

    if (dir == NULL){
        perror("error");
        exit(-1);
    }

    struct dirent *file;

    while ((file = readdir(dir)) != NULL){
        char *new_path;
        new_path = concat("", concat(path, concat("/", file -> d_name)));
        struct stat file_stat;
        lstat(new_path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)){
            if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0){
                atime(new_path, sign, depth - 1, date);
            }
        }

        time_t atime = file_stat.st_atime;
        int diff = difftime(date, atime);

        if ((sign == '-' && diff <= 0) || (sign == '+' && diff >= 0)){
            if (strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0)
            print_file_info(new_path, file_stat);
        } 
    }
    closedir(dir);
}


void read_command(int argc, char *argv[]){
    if (strcmp(argv[3], "-maxdepth") == 0){
        max_depth(argv[2], atoi(argv[4]));
        return;
    }

    if (strcmp(argv[3], "-mtime") == 0){
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        tm.tm_mday -= atoi(argv[5]);
        if (argc > 6){
            if (strcmp(argv[6], "-maxdepth") == 0){
                mtime(argv[2], argv[4][0], atoi(argv[7]), mktime(&tm));
            }
        }else{
            mtime(argv[2], argv[4][0], -1, mktime(&tm));
        }
        return;
    }

    if (strcmp(argv[3], "-atime") == 0){
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        tm.tm_mday -= atoi(argv[5]);
        if (argc > 6){
            if (strcmp(argv[6], "-maxdepth") == 0){
                atime(argv[2], argv[4][0], atoi(argv[7]), mktime(&tm));
            }
        }else{
            atime(argv[2], argv[4][0], -1, mktime(&tm));
        }
        return;
    }
}


int main(int argc, char *argv[]){
    read_command(argc, argv);
}
