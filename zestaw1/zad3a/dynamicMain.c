#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "blocks.h"

void *dl_handle;
typedef void *(*arbitrary)();

void comparePairs(struct Table *mainTab, char *pairs){
    arbitrary fcompareSequence;
    *(void **)(&fcompareSequence) = dlsym(dl_handle, "compareSequence");
    fcompareSequence(pairs);
    arbitrary faddBlockOfEditingOperations;
    *(void **)(&faddBlockOfEditingOperations) = dlsym(dl_handle, "addBlockOfEditingOperations");
    faddBlockOfEditingOperations(mainTab);
}

void removeBlock(struct Table *mainTab, int blockIndex){
    arbitrary fdeleteBlock;
    *(void **)(&fdeleteBlock) = dlsym(dl_handle, "deleteBlock");
    fdeleteBlock(mainTab, blockIndex);
}

void removeOperation(struct Table *mainTab, int blockIndex, int operationIndex){
    arbitrary fdeleteOperation;
    *(void **)(&fdeleteOperation) = dlsym(dl_handle, "deleteOperation");
    fdeleteOperation(mainTab, blockIndex, operationIndex);
}

void readCommands(int argc, char *argv[]){
    arbitrary fconcat;
    *(void **)(&fconcat) = dlsym(dl_handle, "concat");
    struct Table mainTab;
    mainTab.length = 0;
    mainTab.editOpsBlocksLength = NULL;
    mainTab.editOpsBlocks = NULL;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "compare_files") == 0){
            char *pairsSequence = "";
            i++;
            while(i < argc && strcmp(argv[i], "remove_block") != 0 && strcmp(argv[i], "remove_operation") != 0){
                if (strcmp(argv[i - 1], "compare_files") != 0){
                    pairsSequence = fconcat(pairsSequence, " ");
                }
                pairsSequence = fconcat(pairsSequence, argv[i]);
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
}

int main(int argc, char *argv[]){
    dl_handle = dlopen("./blockslib.so", RTLD_LAZY);
    if(!dl_handle){
        printf("!!! %s\n", dlerror());
        return 0;
    }
    readCommands(argc, argv);
    exit(0);
}
