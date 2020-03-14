#include <stdio.h>
#include <stdlib.h>

//concat two string into one
char *concat(const char *str1, const char str2){
    char *linked = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(linked, str1);
    strcat(linked, str2);
    return linked;
}

//compare two text files with diff and save results into tmp.txt
void compareFiles(char *file1, char *file2){
    char *command = concat("diff ", concat(file1, concat(" ", concat(file2, " >> tmp.txt"))));
    system(command);
}

//compare the sequence of files and save the result into tmp.txt
void compareSequence(char *sequence){
    char sequenceTab[strlen(sequence)];
    for(int i = 0; i < strlen(sequence); i++){
        sequenceTab[i] = sequence[i];
    }

    char separator[] = ":";
    char *toCompare;
    system("echo "" > tmp.txt");
    toCompare = strtok(sequenceTab, separator);
    while(toCompare != NULL){
        char file1 = toCompare;
        toCompare = strtok(NULL, separator);
        char *file2 = toCompare;
        toCompare = strtok(NULL, separator);
        compareFiles(file1, file2);
    }
}

//count editing operations in tmp.txt
int countEditingOperations(){
    FILE *tmpContent = fopen("tmp.txt", "r");
    if(tmpContent == NULL){
        printf("tmp.txt  is empty or doesn't exist");
        exit(EXIT_FAILURE);
    }
    int counter = 0;
    char editOps[255];
    while(fgets(editOps, 255, tmpContent)){
        if(isdigit(editOps[0])){
            counter ++;
        }
    }
    fclose(tmpContent);
    return counter;
}

//create block of editing operations and fill it with operations present in tmp.txt
char ** createEditingOperationsBlock(){
    FILE *fopen = ("tmp.txt", "r");
    char editOps[255];
    if(tmpContent == NULL){
        printf("tmp.txt  is empty or doesn't exist");
        exit(EXIT_FAILURE);
    }
    int editingOperationsNo = countEditingOperations();
    char * operation = "NULL";
    char **block = calloc(editingOperationsNo, sizeof(char*));
    int index = 0;
    while(fgets(editOps, 255, tmpContent)){
        if(isdigit(editOps[0])){
            if(i != 0){
                block[index - 1] = operation;
            }
            operation = "";
            operation = concat(operation, editOps);
            index++;
        }
        else{
            operation = concat(operation, editOps);
        }
    }
    block[index - 1] = operation;
    return block;
}

// struct Table{
//     int length;
//     int *editOpsBlocksLength;
//     char ***editOpsBlocks;
// }

//initialize main table
struct Table initTable(){
    struct Table mainTab;
    mainTab.length = 0;
    mainTab.editOpsBlocksLength = NULL;
    mainTab.editOpsBlocks = NULL;
    return mainTab;
}

//create block of editing operations, add operations to it and add the block to main table
int addBlockOfEditingOperations(struct Table *mainTab){
    if(mainTab == NULL){
        printf("null pointer exception");
        exit(EXIT_FAILURE);
    }
    if(mianTab -> length == 0){
        mainTab -> length = 1;
        mainTab -> editOpsBlocksLength = calloc(1, sizeof(int));
        mainTab -> editOpsBlocksLength[0] = countEditingOperations();
        mainTab -> editOpsBlocks = calloc(1, sizeof(char**));
        mainTab -> editOpsBlocks[0] = createEditingOperationsBlock();
        return 0;
    }
    mainTab -> length++;
    int length = mainTab -> length;
    mainTab -> editOpsBlocksLength = realloc(mainTab -> editOpsBlocksLength, length * sizeof(int));
    mainTab -> editOpsBlocksLength[length - 1] = countEditingOperations();
    mainTab -> editOpsBlocks = realloc(mainTab -> editOpsBlocks, length * sizeof(char**));
    mainTab -> editOpsBlocks[length - 1] = createEditingOperationsBlock();
    return length - 1;
}

void deleteOperation(struct Table *mainTab, int blockIndex, int operationIndex){
    if(mainTab == NULL){
        printf("null pointer exception");
        exit(EXIT_FAILURE);
    }
    if(blockIndex > mainTab -> length || blockIndex < 0){
        printf("block index is out of range");
        exit(EXIT_FAILURE);
    }
    if(operationIndex > mainTab -> editOpsBlocksLength[blockIndex] - 1 || operationIndex < 0){
        printf("operation index is out of range");
        exit(EXIT_FAILURE);
    }
    char ** block = mainTab -> editOpsBlocks[blockIndex];
    free(block[operationIndex]);
    block[operationIndex] = NULL;
}

void deleteBlock(struct Table *mainTable, int blockIndex){
    if(mainTab == NULL){
        printf("null pointer exception");
        exit(EXIT_FAILURE);
    }
    char **block = mainTab -> editOpsBlocks[blockIndex];
    int blockLength = mainTab -> editOpsBlocksLength[blockIndex];
    mainTab -> editOpsBlocksLength[blockIndex]--;
    for(int i = 0; i < blockLength; i++){
        free(block[i]);
    }
    free(block);
    block = NULL;
}