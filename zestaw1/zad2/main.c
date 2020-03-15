#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "blocks.h"

void comparePairs(struct Table *mainTab, char *pairs){
    clock_t begin, end;
    compareSequence(pairs);
    begin = clock();
    addBlockOfEditingOperations(mainTab);
    end = clock();
    printf("pair comparison time: %f\n", (((double) (end - begin))/ CLOCKS_PER_SEC));
}

void removeBlock(struct Table *mainTab, int blockIndex){
    clock_t begin, end;
    begin = clock();
    deleteBlock(mainTab, blockIndex);
    end = clock();
    printf("freeing memory time: %f\n", (((double) (end - begin)) / CLOCKS_PER_SEC));
}

void removeOperation(struct Table *mainTab, int blockIndex, int operationIndex){
    clock_t begin, end;
    begin = clock();
    deleteOperation(mainTab, blockIndex, operationIndex);
    end = clock();
    printf("deleting operation time: %f\n", (((double) (end - begin)) / CLOCKS_PER_SEC));
}

void readCommands(int argc, char *argv[]){
    struct Table mainTab;
    mainTab = initTable();
    bool flag = false;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "compare_files") == 0){
            flag = true;
            continue;
        }
        if(strcmp(argv[i], "remove_block") == 0){
            flag = false;
            removeBlock(&mainTab, (int)(argv[i+1][0] - '0'));
            continue;
        }
        if(strcmp(argv[i], "remove_operation") == 0){
            flag = false;
            removeOperation(&mainTab, (int)(argv[i+1][0] - '0'), (int)(argv[i+2][0] - '0'));
            i += 2;
            continue;
        }
        if(flag){
            comparePairs(&mainTab, argv[i]);
            continue;
        }
    }
    deleteTable(&mainTab);
}

int main(int argc, char *argv[]){
    readCommands(argc, argv);
    exit(0);
}
