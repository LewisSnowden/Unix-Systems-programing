
// Created by lewis on 5/4/20.

#ifndef USP_ASSIGNMENT_SYMBOLTABLE_H
#define USP_ASSIGNMENT_SYMBOLTABLE_H
/******************************************************************************************************
Struct name: SymbolTableEntry
Purpose: to be the Entry in Symbol Table
********************************************************************************************************/
typedef struct SymbolTableEntry{
    char* key;
    int state; //0= never used, 1= used, -1 formally used
    void* value;

}SymbolTableEntry;


/******************************************************************************************************
Struct name: SymbolTable
Purpose: struct of SymbolTable
********************************************************************************************************/
typedef struct {
    SymbolTableEntry* entries;
    int count;  /*stores how many nodes are in the list */
    int size;
    int numResize;
}SymTab;

SymTab* new();
void freeTable(SymTab* oSymTab);
int contains(SymTab* oSymTab, const char* key);
void* get(SymTab* oSymTab, const char* key);
int put(SymTab* oSymTab, const char* key, const void* value);
void myStrCpy(char* dest, char* src);
int removeEntry(SymTab* oSymTab, const char *key);
SymTab* resize(SymTab* oSymTab);
int hash(SymTab* oSymTab,const char* key);
int myStrlen(char* inString);
int myStrEqual(char* char1, char* char2);
int myNextPrime(int inNumber);
float mySqrt(int inNumber);
int openFile(SymTab* oSymTab);
int myAtoi(char* inStr);
char* myStrSplitOnComma(char* inString,int index);
void printToError(char* string);
#endif //USP_ASSIGNMENT_SYMBOLTABLE_H
