#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "blocks.h"

void comparePairs(struct Table *mainTab, char *pairs){
    compareSequence(pairs);
    addBlockOfEditingOperations(mainTab);
}

void removeBlock(struct Table *mainTab, int blockIndex){
    deleteBlock(mainTab, blockIndex);
}

void removeOperation(struct Table *mainTab, int blockIndex, int operationIndex){
    deleteOperation(mainTab, blockIndex, operationIndex);
}

void readCommands(int argc, char *argv[]){
    struct Table mainTab;
    mainTab = initTable();

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "compare_files") == 0){
            char *pairsSequence = "";
            i++;
            while(i < argc && strcmp(argv[i], "remove_block") != 0 && strcmp(argv[i], "remove_operation") != 0){
                if (strcmp(argv[i - 1], "compare_files") != 0){
                    pairsSequence = concat(pairsSequence, " ");
                }
                pairsSequence = concat(pairsSequence, argv[i]);
                i++;
            }
            comparePairs(&mainTab, pairsSequence);
            continue;
        }
        if(strcmp(argv[i], "remove_block") == 0){
            removeBlock(&mainTab, (int)(argv[i+1][0] - '0'));
            i++;
            continue;
        }
        if(strcmp(argv[i], "remove_operation") == 0){
            removeOperation(&mainTab, (int)(argv[i+1][0] - '0'), (int)(argv[i+2][0] - '0'));
            i += 2;
            continue;
        }
    }
    deleteTable(&mainTab);
}

int main(int argc, char *argv[]){
    readCommands(argc, argv);
    exit(0);
}
