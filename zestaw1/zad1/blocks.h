struct Table{
    int length;
    int *editOpsBlocksLength;
    char ***editOpsBlocks;
}

char *concat(const char *str1, const char str2);
void compareFiles(char *file1, char *file2);
void compareSequence(char *sequence);
int countEditingOperations();
char **createEditingOperationsBlock();
struct Table initTable();
int addBlockOfEditingOperations(struct Table *mainTab);
void deleteOperation(struct Table *mainTab, int blockIndex, int operationIndex);
void deleteBlock(struct Table *mainTable, int blockIndex);


