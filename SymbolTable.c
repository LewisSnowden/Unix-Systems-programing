#include "SymbolTable.h"
#include <sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#define SIZEOFTABLE 24
#define SIZEOFENTRY 24
#define BUFSIZE 16000



/******************************************************************************************************
Function: new
Purpose: creates new Hash Table
Imports:
Exports: exports new hash table
********************************************************************************************************/
SymTab* new()
{
    SymTab* table;
    table = mmap(NULL,SIZEOFTABLE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0); //allocating memory to table
    table->entries = mmap(NULL,SIZEOFENTRY*769,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);//allocating memory to entry pointer
    table->count=0;  //initialising the variables
    table->size=769;
    table->numResize=0;
    for(int ii=0;ii<769;ii++)  //making sure all the states are set to 0
    {
        (*(table->entries+ii)).state=0;
    }
    return table;
}

/******************************************************************************************************
Function: freeTable
Purpose: frees all memory used by table
Imports: oSymTab (SymTab*) imports a hash table
Exports: none
********************************************************************************************************/
void freeTable(SymTab* oSymTab)
{
    for(int ii = 0;ii<oSymTab->size;ii++)
    {
        if((*(oSymTab->entries+ii)).state==1) //if the entry actully contains data must free the key
        {
            munmap((*(oSymTab->entries+ii)).key,myStrlen((*(oSymTab->entries+ii)).key)+1); //freeing the keys
        }
    }
    munmap(oSymTab->entries,SIZEOFENTRY*oSymTab->size); //freeing the entry pointers
    munmap(oSymTab,SIZEOFTABLE); //freeing the table
}

/******************************************************************************************************
Function: contains
Purpose: checks if the table contains a value
Imports: oSymTab (SymTab*) the symbol table, key (char*) the key
Exports: 0 for not found, 1 for found
********************************************************************************************************/
int contains(SymTab* oSymTab, const char* key)
{
    if(oSymTab==NULL) //checking for null table
    {
        printToError("Error in contains, oSymTab is NULL");
        return 0;
    }
    else if(key==NULL) //checking for null key
    {
        printToError("Error in contains, key is NULL");
        return 0;
    }
    int hashIdx = hash(oSymTab,key);
    int origIdx = hashIdx;
    int found = 0;
    int giveUp = 0;
    while((found != 1)&&(giveUp !=1)) //continues till either its found or gives up
    {
        if((*(oSymTab->entries+hashIdx)).state ==0) //if the state = 0 then the entry does not exist for corosponding key
        {
            giveUp = 1;
        }
        else if((*(oSymTab->entries+hashIdx)).state==1)  //making sure its only checking for entries that are actully there
        {
            if(myStrEqual((*(oSymTab->entries+hashIdx)).key,key)==0) //do the keys match
            {
                found = 1;
            }
            else
            {
                hashIdx = (hashIdx+1)%oSymTab->size; //if they dont match go to next entry, might be a linear probe aka duplicate hashIdx
            }
        }
        else //if an entry is -1 might be previous entry but has been removed to check next
        {
            hashIdx = (hashIdx+1)%oSymTab->size;
            if(hashIdx==origIdx)
            {
                giveUp = 1;
            }
        }
    }

    if(found==0)
    {
        printToError("Error in contains, Could not find entry in table");
        return 0;

    }
    return 1; //change later should be return the value at hashIdx
}

/******************************************************************************************************
Function: get
Purpose: retrives value from table from a given key
Imports: oSymTab (SymTab*) the symbol table, key (char*) the key
Exports: corisponding value to the imported key
********************************************************************************************************/
void* get(SymTab* oSymTab, const char* key)
{
    if(oSymTab==NULL) //checking null table
    {
        printToError("Error in get, oSymTab is NULL");
        return NULL;
    }
    else if(key==NULL) //checking
    {
        printToError("Error in get, key is NULL");
        return NULL;
    }
    int hashIdx = hash(oSymTab,key);
    int origIdx = hashIdx;
    int found = 0;
    int giveUp = 0;
    while((found != 1)&&(giveUp !=1)) //continues while the value is not found or
    {
        if((*(oSymTab->entries+hashIdx)).state ==0) //if the state is equal to 0 then the key does not exist in table
        {
            giveUp = 1;
        }
        else if((*(oSymTab->entries+hashIdx)).state==1)  //making sure its only checking for entries that are actully there
        {
            if(myStrEqual((*(oSymTab->entries+hashIdx)).key,key)==0) //do the key in table and imported keys match
            {
                found = 1;
            }
            else
            {
                hashIdx = (hashIdx+1)%oSymTab->size; //if it doesnt equal keep searching
            }
        }
        else
        {
            hashIdx = (hashIdx+1)%oSymTab->size;
            if(hashIdx==origIdx)
            {
                giveUp = 1;
            }
        }
    }

    if(found==0)
    {
        printToError("Error in get could not retrieve");
        return NULL;
    }
    return (*(oSymTab->entries+hashIdx)).value; //change later should be return the value at hashIdx
}

/******************************************************************************************************
Function: put
Purpose: inserts new entry into the hashtable
Imports: oSymTab (SymTab*) the symbol table, key (char*) the key, value (void*) the value
Exports:
********************************************************************************************************/
int put(SymTab* oSymTab, const char* key, const void* value)
{
    if(oSymTab ==NULL) //checking for Null table
    {
        printToError("Error in PUT, SymTab is null");
        return 0;
    }
    else if(key==NULL) //checking for null key
    {
        printToError("Error in PUT, key is null");
        return 0;
    }
    if(oSymTab->count != 98317) //not at max capacity
    {
        char* originalChar = key;
        char* newKey;
        int ii=0;
        newKey = mmap(NULL,myStrlen(key)+1,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0); //asigning memory to key copy
        myStrCpy(newKey,key); //copying key into the copy of key to insert into table
        SymbolTableEntry newEntry;
        newEntry.value = value;
        newEntry.key = newKey;
        newEntry.state = 1;
        int hashIdx = hash(oSymTab,newEntry.key);

        if((*(oSymTab->entries+hashIdx)).state==0) //if no entry is in the wanted slot insert it
        {
            (*(oSymTab->entries+hashIdx))=newEntry;
        }
        else
        {
            while((*(oSymTab->entries+hashIdx+ii)).state ==1) //while there is an entry, cannot insert
            {
                ii++;
                if(hashIdx == (oSymTab->size)) //wrap around when the end of table is reached
                {
                    hashIdx=0;
                    ii=0;
                }
            }
            (*(oSymTab->entries+hashIdx+ii))=newEntry; //inserting entry into table

        }
        oSymTab->count = oSymTab->count + 1; //increasing the count

        if((float)oSymTab->count/(float)oSymTab->size>=0.7) //checking resize condition
        {
            oSymTab = resize(oSymTab);
        }

        return 1;
    }
    else
    {
        printToError("Error in PUT, maximum capacity reached");
        return 0;
    }
}
/******************************************************************************************************
Function: myStrCpy
Purpose: copies the contents from src pointer into the dest pointer
Imports: dest (char*) destintion location, srs(char*) source location
Exports: none
********************************************************************************************************/
void myStrCpy(char* dest, char* src)
{
    int ii = 0;
    while (*(src+ii) != '\0') //while not the end of the string
    {

        *(dest+ii) = *(src+ii); //copies character from src and puts it into the destination
        ii++;
    }
    *(dest+ii) = '\0';
}

/******************************************************************************************************
Function: removeEntry
Purpose: handles the removal of entries from table
Import: oSymTab (SymTab*) the symbol table, key (char*) the key
Export:0 if removal un sucessful, 1 for successfull removal
********************************************************************************************************/
int removeEntry(SymTab* oSymTab, const char *key)
{
    int hashIdx;
    int origIdx;
    int found = 0;
    int giveUp = 0;
    if(oSymTab==NULL) //checking for null table
    {
        printToError("Error in remove, oSymTab is NULL");
        return 0;
    }
    else if(key==NULL) //checking for null key
    {
        printToError("Error in remove, key is NULL");
        return 0;
    }
    else if(oSymTab->count==0) //checks if table is empty, cant remove if its empty
    {
        printToError("Cannot call remove when table is empty");
        return 0;
    }
    hashIdx = hash(oSymTab,key);
    origIdx = hashIdx;
    while((found != 1)&&(giveUp !=1)) //continues while entry is not found and hasnt given up
    {

        if((*(oSymTab->entries+hashIdx)).state ==0) //if the entry state equal to 0 then it doesnt exist in table
        {
            giveUp = 1;
        }
        else if((*(oSymTab->entries+hashIdx)).state==1)  //making sure its only checking for entries that are actully there
        {
            if(myStrEqual((*(oSymTab->entries+hashIdx)).key,key)==0)
            {
                found = 1;
            }
            else
            {
                hashIdx = (hashIdx+1)%oSymTab->size;
            }
        }
        else //if the state is -1 means that its a deleted entry so it might be the next entry
        {
            hashIdx = (hashIdx+1)%oSymTab->size;
            if(hashIdx==origIdx)
            {
                giveUp = 1;
            }
        }
    }
    if(found==0)
    {
        printToError("Error in remove, could not find key in table");
        return 0;

    }
    munmap((*(oSymTab->entries+hashIdx)).key,1); //freeing key before removal
    (*(oSymTab->entries+hashIdx)).key = NULL;
    (*(oSymTab->entries+hashIdx)).state = -1;   //setting state to -1 to indicate previous entry
    (*(oSymTab->entries+hashIdx)).value = NULL;
    oSymTab->count = oSymTab->count -1;

    return 1;
}

/******************************************************************************************************
Function: resize
Purpose: once table has reached capacity will increase number of entries
Import: oSymTab (SymTab*) the symbol table
Export: new symbol table
********************************************************************************************************/
SymTab* resize(SymTab* oSymTab)
{

    SymbolTableEntry* oldTable;
    int newSize = openFile(oSymTab);
    if(newSize ==-1)
    {
        printToError("error in openFile resize not peformed");
        return oSymTab;
    }
    int oldSize;
    SymbolTableEntry* tempEntries=mmap(NULL,SIZEOFENTRY*newSize,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0); //alocating memeory for new entries
    for(int jj=0;jj<newSize;jj++)
    {
        (*(tempEntries+jj)).state=0; //initialsing all states to be 0
    }
    oldTable = oSymTab->entries; //making copy of old entries
    oldSize = oSymTab->size; //old size
    oSymTab->entries = tempEntries; //giving new capacity to table
    oSymTab->size = newSize;
    oSymTab->count =0;
    for(int ii=0;ii<oldSize;ii++) //goes through all entries of old table
    {

        if((*(oldTable+ii)).state==1) //if there is an actual entry present
        {
            put(oSymTab,(*(oldTable+ii)).key,(*(oldTable+ii)).value); //put value from old entries into new
        }
    }
    for(int ii = 0;ii<oldSize;ii++)
    {
        if((*(oldTable+ii)).state==1)
        {
            munmap((*(oldTable+ii)).key,myStrlen((*(oldTable+ii)).key)); //de allocated memeory from old keys
        }
    }
    oSymTab->numResize = oSymTab->numResize+1; //inreasing the number of times the table has been resized
    return oSymTab;
}
/******************************************************************************************************
Function: hash
Purpose: returns the hash of imported string
Imports: oSymTab (SymTab*) the symbol table, key (char*) the key
Exports: export (int) the hash of the string
note: hash code used from hash table lecture slides(slide 27)
********************************************************************************************************/
int hash(SymTab* oSymTab,const char* key)
{

    int hashIdx=0;
    int export;
    int ii;
    char* originalLocation = key;
    for(ii=0;ii<myStrlen(key);ii++)
    {
        hashIdx = (31*hashIdx)+*key;
        key++;
    }
    if((hashIdx % oSymTab->size)<0 )
    {
        export = hashIdx % oSymTab->size + oSymTab->size;
    }
    else
    {
        export = hashIdx % oSymTab->size;
    }
    return export;
}
/******************************************************************************************************
Function: myStrlen
Purpose: returns length of string
Imports: inString(char*) the string to measure the length of
Exports: stringLength(int) the length of string
********************************************************************************************************/
int myStrlen(char* inString)
{
    int stringLength = 0;
    while(*inString != '\0')
    {
        inString++;
        stringLength++;
    }
    return stringLength;
}
/******************************************************************************************************
Function: myStrEqual
Purpose: checks if strings are equal
Imports: char1 (char*), char2(char*)
Exports: 0 if strings are the same
********************************************************************************************************/
int myStrEqual(char* char1, char* char2)
{
    int isEqual =0;
    while(( *char1 != '\0' && *char2 != '\0' )&&(isEqual==0))
    {
        if(*char1 != *char2)
        {
            isEqual = 1;
        }
        char1++;
        char2++;
    }
    if(*char1==*char2)
    {
        isEqual = 0;
    } else
    {
        isEqual=1;
    }
    return isEqual; // strings are identical
}
/******************************************************************************************************
Function: printToError
Purpose: writes to standard error the given string
Imports: string (char*) message to write
Exports: none
********************************************************************************************************/
void printToError(char* string)
{
    write(2,string,myStrlen(string));
}

/******************************************************************************************************
Function: openFile
Purpose: opens the file and returns number based on how many resizes have occured
Imports: oSymTab (SymTab*) the symbol table
Exports: number(int) the entry capacity from file
********************************************************************************************************/
int openFile(SymTab* oSymTab)
{
    int numberFileFD;
    int number;
    char buffer[BUFSIZE]; //create buffer with a resonable ammount of characters
    char* str;
    numberFileFD = open("test.txt", O_RDONLY | O_CREAT, 0644); //opening file
    if(numberFileFD!=-1) //if there is no  error in opening file
    {
        read(numberFileFD,&buffer,BUFSIZE); //read the line
        str = myStrSplitOnComma(buffer,oSymTab->numResize+1); //retriving the new size of file
        number= myAtoi(str);
        if(close(numberFileFD)==1)
        {
            printToError("Error in open file, could not close it");
            return -1;
        }
        munmap(str,6); //de allocating space for string
        return number;
    }
    else
    {
        printToError("Error in open file, could not open the file");
        return -1;
    }

}
/******************************************************************************************************
Function: myAtoi
Purpose: converts char* to int
Imports:string(char*) the string to convert to an int
Exports:result (int) number
********************************************************************************************************/
int myAtoi(char* string)
{
    int result =0;
    int ii=0;
    while(*(string+ii) != '\0')
    {
        result = result * 10 + *(string+ii) - '0';
        ii++;
    }
    return result;
}
/******************************************************************************************************
Function: myStrSplitOnComma
Purpose: to split line read from file and return number based on index number given
Imports: string(char*) line to retrive number from , index(int) position to retrive number from
Exports: char* number from file in char* form
Note:
 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317
 ^---- index 0
        ^---- index 1 etc etc
********************************************************************************************************/
char* myStrSplitOnComma(char* string,int index)
{

    int delimiterCount=0;
    char* duplicateInString = string;
    char* subString = mmap(NULL,6,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);//can be 5 as max size is 98317 which is 5 characters long and '\0' char
    char currChar;
    int ii=0;
    while(delimiterCount<index+1)  //repeats until required number is reached
    {
        currChar = *duplicateInString;  //holding value of current character in line
        if(index==0)
        {
            while(currChar != ',')//untill the comma is reached record the characters
            {

                *(subString+ii)=currChar; //adding char to output array
                duplicateInString++; //moving to next character
                currChar=*duplicateInString; //checking next character

                ii++;
            }

            *(subString+ii)='\0'; //adding to end of char array

        }
        else
        {
            if(currChar==',')   //once comma is reached
            {
                delimiterCount++;  //increase number of commas that it has seen
                if(delimiterCount==index)  //if the number of commas is equal to the index then the next charactrs will be the required number
                {
                    duplicateInString++; //skipping over comma
                    duplicateInString++; //skipping over space
                    currChar=*duplicateInString; //capture next character
                    while(currChar != ','&&ii<5) //second boolean statment is to deal with last number because there are no more commas
                    {

                        *(subString+ii)=currChar;
                        duplicateInString++;
                        currChar=*duplicateInString;

                        ii++;
                    }

                    *(subString+ii)='\0';

                    delimiterCount++;
                }
            }
        }


        duplicateInString++;
    }
    return subString;
}