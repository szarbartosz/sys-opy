#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


void print_file_info(const char *path, struct stat file_stat){
    char *file_type;

    if (S_ISREG(file_stat.st_mode)){
        file_type = "file";
    } else if (S_ISDIR(file_stat.st_mode)){
        file_type = "dir";
    } else if (S_ISCHR(file_stat.st_mode)){
        file_type = "char";
    } else if (S_ISBLK(file_stat.st_mode)){
        file_type = "block dev";
    } else if (S_ISFIFO(file_stat.st_mode)){
        file_type = "fifo";
    } else if (S_ISLNK(file_stat.st_mode)){
        file_type = "slink";
    } else if (S_ISSOCK(file_stat.st_mode)){
        file_type = "socket";
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


int maxdepth;
char *command;
char sign;
time_t date;


int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if (ftwbuf -> level > maxdepth){
        return 0;
    }

    if (strcmp(command, "-maxdepth") == 0)
    {
        print_file_info(fpath, *sb);
        return 0;
    }

    if (strcmp(command, "-mtime") == 0){
        time_t modtime = sb -> st_mtime;
        int diff = difftime(date, modtime);

        if ((sign == '-' && diff <= 0) || (sign == '+' && diff >= 0)){
            print_file_info(fpath, *sb);
        }
        return 0;
    }

    if (strcmp(command, "-atime") == 0){
        time_t atime = sb -> st_atime;
        int diff = difftime(date, atime);

        if ((sign == '-' && diff <= 0) || (sign == '+' && diff >= 0)){
            print_file_info(fpath, *sb);
        }
        return 0;
    }
    return 0;
}


void read_command(int argc, char *argv[]){
    char *path = argv[2];
    command = argv[3];

    if (strcmp(command, "-maxdepth") == 0){
        maxdepth = atoi(argv[4]);
        
    } else if (strcmp(command, "-mtime") == 0 || strcmp(command, "-atime") == 0){
        sign = argv[4][0];
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        tm.tm_mday -= atoi(argv[5]);
        date = mktime(&tm);
        if (argc > 6){
            maxdepth = atoi(argv[7]);
        } else {
            maxdepth = -1;
        }
    }
    nftw(path, fn, FTW_D, FTW_PHYS);
}


int main(int argc, char *argv[]){
    read_command(argc, argv);
}