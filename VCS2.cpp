 #ifdef _MSC_VER
 #define _CRT_SECURE_NO_WARNINGS
 #endif
 #pragma warning(disable : 4996)
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>    //windows
#include<iostream>
//hole in the file is potential gap in the file where the data is not there but can be further 
//hole file is the pagefile.sys in windows
//hard disk contains the magnestism and the RAM has the electricity
// hard disk madhli chotisi jaga which is formatted as the RAM is called as the swap partition ie part of the hard disk formatted as the ram ram sarkha asta but it is not like the RAM


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include<cstring>//done
#include <locale.h> //done
#include<sys/types.h>
#include<sys/stat.h>
#include<windows.h> //done
#include<tlhelp32.h>//done
#include <assert.h> //done
//#include "pch.h"
#include <algorithm>//done
#include <vector>//done


#define MAXINODE 50      //50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 2048  //2048

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

#define UNMODIFIED 20
#define MODIFIED 21
#define STAGED 22
#define TRACKED 23

typedef struct superblock
{
    int Totalnodes;
    int FreeInode;
} SUPERBLOCK, * PSUPERBLOCK;


typedef struct inode
{
    char FileName[50];
    int inodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char* Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;
    int stage;
    DWORD hr;
    DWORD min;
    DWORD date;
    DWORD month;
    struct inode* next;
} INODE, * PINODE, ** PPINODE;


typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;
    PINODE ptrinode;
}FILETABLE, * PFILETABLE;


typedef struct uftd
{
    PFILETABLE ptrfiletable;
}UFTD;

UFTD UFTDArr[MAXINODE];

SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;


void man(char* name)
{
    if (name == NULL)
        return;

    if (strcmp(name, "create") == 0)
    {
        printf("Description : Used to create new regular files\n");
        printf("usage : create File_name Permission\n");
    }
    else if (strcmp(name, "read") == 0)
    {
        printf("Description : Used to read from regular files\n");
        printf("usage : read File_name No_of_bytes_to_read\n");
    }
    else if (strcmp(name, "write") == 0)
    {
        printf("Description : Used to write in regular files\n");
        printf("usage : write File_name\n After this enter the data that we want to write \n");
    }
    else if (strcmp(name, "ls") == 0)
    {
        printf("Description : Used to list all information of files\n");
        printf("usage : ls\n");
    }
    else if (strcmp(name, "stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("usage : stat File_name\n");
    }
    else if (strcmp(name, "fstat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("usage : stat File_Descriptor\n");
    }
    else if (strcmp(name, "truncate") == 0)
    {
        printf("Description : Used to remove data from file\n");
        printf("usage : truncate File_name\n");
    }
    else if (strcmp(name, "open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("usage : open File_name mode\n");
    }
    else if (strcmp(name, "close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("usage : close File_name\n");
    }
    else if (strcmp(name, "closeall") == 0)
    {
        printf("Description : Used to close all opened files\n");
        printf("usage : closeall\n");
    }
    else if (strcmp(name, "lseek") == 0)
    {
        printf("Description : Used to change file offset\n");
        printf("usage : lseek File_Name ChangeInOffset StartPoint\n");
    }
    else if (strcmp(name, "rm") == 0)
    {
        printf("Description : Used to delete the file\n");
        printf("usage : rm File_Name\n");
    }
    else
    {
        printf("ERROR : No manual entry available.\n");
    }

}

void DisplayHelp()
{
    printf("ls : List out all the files \n");
    printf("clear : To clear the console \n");
    printf("open : To open the file \n");
    printf("close : TO close the file \n");
    printf("closeall : To close all the files \n");
    printf("read : To Read the contents of the file \n");
    printf("write : To write the contents of the file \n");
    printf("exit : To terminate the file system \n");
    printf("stat : To Display information of file using name \n");
    printf("fstat : To Display information of file using file descriptor \n");
    printf("truncate : To remove all the data from file \n");
    printf("rm : To delete the file \n");
}

int GetFDFromName(char* name)
{
    int i = 0;
    while (i < MAXINODE)
    {
        if (UFTDArr[i].ptrfiletable != NULL)
        {
            if (strcmp((UFTDArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
            {
                break;
            }
        }
        i++;
    }

    if (i == MAXINODE)
    {
        return -1;
    }

    else
    {
        return i;
    }
}


PINODE Get_Inode(char* name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
    {
        return NULL;
    }

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
        {
            break;
        }

        temp = temp->next;
    }
    return temp;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while (i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;
        newn->stage = UNMODIFIED;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->inodeNumber = i;

        if (temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

void InitialiseSuperBlock()
{
    //printf("");
    int i = 0;

    while (i < MAXINODE)
    {
        UFTDArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.Totalnodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}


int CreateFile(char* name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if ((name == NULL) || (permission == 0) || (permission > 3))
    {
        return -1;
    }

    if (SUPERBLOCKobj.FreeInode == 0)
    {
        return -2;
    }

    (SUPERBLOCKobj.FreeInode)--;

    if (Get_Inode(name) != NULL)
    {
        return -3;
    }

    while (temp != NULL)
    {
        if (temp->FileType == 0)
        {
            break;
        }
        temp = temp->next;
    }

    while (i < 50)
    {
        if (UFTDArr[i].ptrfiletable == NULL)
        {
            break;
        }

        i++;
    }

    UFTDArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFTDArr[i].ptrfiletable->count = 1;
    UFTDArr[i].ptrfiletable->mode = permission;
    UFTDArr[i].ptrfiletable->readoffset = 0;
    UFTDArr[i].ptrfiletable->writeoffset = 0;

    UFTDArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFTDArr[i].ptrfiletable->ptrinode->FileName, name);
    UFTDArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFTDArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFTDArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFTDArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFTDArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFTDArr[i].ptrfiletable->ptrinode->stage = TRACKED;
    UFTDArr[i].ptrfiletable->ptrinode->permission = permission;
    UFTDArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFILESIZE);

    printf("\n The file is being tracked by Amit Version Control System.........");
    printf("\n");
    return i;

}


int rm_File(char* name)
{
    int fd = 0;
    fd = GetFDFromName(name);
    if (fd == -1)
    {
        return -1;
    }

    (UFTDArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if (UFTDArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFTDArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFTDArr[fd].ptrfiletable);
    }

    UFTDArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;

}


int ReadFile(int fd, char* arr, int isize)
{
    int read_size = 0;

    if (UFTDArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if ((UFTDArr[fd].ptrfiletable->mode != READ) && (UFTDArr[fd].ptrfiletable->mode != READ + WRITE))
    {
        return -2;
    }

    if ((UFTDArr[fd].ptrfiletable->ptrinode->permission != READ) && (UFTDArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE))
    {
        return -2;
    }

    if (UFTDArr[fd].ptrfiletable->readoffset == UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize)
    {
        return -3;
    }

    if ((UFTDArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
    {
        return -4;
    }

    read_size = (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFTDArr[fd].ptrfiletable->readoffset);

    if (read_size < isize)
    {
        strncpy(arr, (UFTDArr[fd].ptrfiletable->ptrinode->Buffer) + (UFTDArr[fd].ptrfiletable->readoffset), read_size);

        UFTDArr[fd].ptrfiletable->readoffset = UFTDArr[fd].ptrfiletable->readoffset + read_size;
    }

    else
    {
        strncpy(arr, (UFTDArr[fd].ptrfiletable->ptrinode->Buffer) + (UFTDArr[fd].ptrfiletable->readoffset), isize);

        UFTDArr[fd].ptrfiletable->readoffset = UFTDArr[fd].ptrfiletable->readoffset + isize;
        UFTDArr[fd].ptrfiletable->ptrinode->stage = STAGED;
        printf("\n This repository is tracked by Amit's Version Control System \n");
        printf("\n");
    }

    return isize;
}


int WriteFile(int fd, char* arr, int isize)
{
    if ((UFTDArr[fd].ptrfiletable->mode != WRITE) && (UFTDArr[fd].ptrfiletable->mode != READ + WRITE))
    {
        return -1;
    }

    if ((UFTDArr[fd].ptrfiletable->ptrinode->permission != WRITE) && (UFTDArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE))
    {
        return -1;
    }

    if ((UFTDArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
    {
        return -2;
    }

    if ((UFTDArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
    {
        return -3;
    }

    strncpy((UFTDArr[fd].ptrfiletable->ptrinode->Buffer) + (UFTDArr[fd].ptrfiletable->writeoffset), arr, isize);

    (UFTDArr[fd].ptrfiletable->writeoffset) = (UFTDArr[fd].ptrfiletable->writeoffset) + isize;

    (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;
    
    UFTDArr[fd].ptrfiletable->ptrinode->stage = MODIFIED;
    printf("\n This repository is tracked by Amit's Version Control System \n");
    printf("\n");

    return isize;
}


int OpenFile(char* name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if ((name == NULL) || (mode <= 0))
    {
        return -1;
    }

    temp = Get_Inode(name);

    if (temp == NULL)
    {
        return -2;
    }

    if (temp->permission < mode)
    {
        return -3;
    }

    while (i < 50)
    {
        if (UFTDArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFTDArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if (UFTDArr[i].ptrfiletable == NULL)
    {
        return -1;
    }

    UFTDArr[i].ptrfiletable->count = 1;
    UFTDArr[i].ptrfiletable->mode = mode;
    if (mode == READ + WRITE)
    {
        UFTDArr[i].ptrfiletable->readoffset = 0;
        UFTDArr[i].ptrfiletable->writeoffset = 0;
    }

    else if (mode == READ)
    {
        UFTDArr[i].ptrfiletable->readoffset = 0;
    }

    else if (mode == WRITE)
    {
        UFTDArr[i].ptrfiletable->writeoffset = 0;
    }

    UFTDArr[i].ptrfiletable->ptrinode = temp;
    (UFTDArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

}

void CloseFileByFileDescriptor(int fd)
{
    UFTDArr[fd].ptrfiletable->ptrinode->stage = UNMODIFIED+STAGED;
    UFTDArr[fd].ptrfiletable->readoffset = 0;
    UFTDArr[fd].ptrfiletable->writeoffset = 0;
    (UFTDArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;

    printf("\n This repository is unmodified and staged by Amit's Version Control System \n");
    printf("\n");

}


int CloseFileByName(char* name)
{
    int i = 0;
    i = GetFDFromName(name);

    if (i == -1)
    {
        return -1;
    }
    UFTDArr[i].ptrfiletable->ptrinode->stage = UNMODIFIED + STAGED;
    UFTDArr[i].ptrfiletable->readoffset = 0;
    UFTDArr[i].ptrfiletable->writeoffset = 0;
    (UFTDArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
    printf("\n This repository is unmodified and staged by Amit's Version Control System \n");
    printf("\n");
    return 0;

}

void freeNodes()
{
    PINODE temp = head;
    int i = 0;
    while (i < MAXINODE)
    {
        if (UFTDArr[i].ptrfiletable != NULL)
        {
            free(UFTDArr[i].ptrfiletable->ptrinode->Buffer);
            free(UFTDArr[i].ptrfiletable);
        }
        head = temp->next;
        free(temp);
        temp = head;
        i++;
    }
    i = 0;
    while (i < MAXINODE)
    {
        UFTDArr[i].ptrfiletable = NULL;
        i++;
    }
}


void CloseAllFile()
{
    int i = 0;
    while (i < 50)
    {
        if (UFTDArr[i].ptrfiletable != NULL)
        {
            UFTDArr[i].ptrfiletable->ptrinode->stage = UNMODIFIED + STAGED;
            UFTDArr[i].ptrfiletable->readoffset = 0;
            UFTDArr[i].ptrfiletable->writeoffset = 0;
            (UFTDArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            printf("\n This repository is unmodified and staged by Amit's Version Control System \n");
            printf("\n");
            break;
        }
        i++;
    }

    //freeNodes();
}


int LseekFile(int fd, int size, int from)
{
    if ((fd < 0) || (from > 2))
    {
        return -1;
    }

    if (UFTDArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if ((UFTDArr[fd].ptrfiletable->mode == READ) || (UFTDArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if (from == CURRENT)
        {
            if (((UFTDArr[fd].ptrfiletable->readoffset) + size) > (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            if (((UFTDArr[fd].ptrfiletable->readoffset) + size) < 0)
            {
                return -1;
            }

            (UFTDArr[fd].ptrfiletable->readoffset) = (UFTDArr[fd].ptrfiletable->readoffset) + size;


        }
        else if (from == START)
        {
            if (size > (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }

            if (size < 0)
            {
                return -1;
            }

            (UFTDArr[fd].ptrfiletable->readoffset) = size;
        }

        else if (from == END)
        {
            if (((UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if (((UFTDArr[fd].ptrfiletable->readoffset) + size) < 0)
            {
                return -1;
            }

            (UFTDArr[fd].ptrfiletable->readoffset) = ((UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) + size);

        }

    }

    else if ((UFTDArr[fd].ptrfiletable->mode == WRITE))
    {
        if (from == CURRENT)
        {
            if (((UFTDArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if (size < 0)
            {
                return -1;
            }

            if (size > (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            }

            (UFTDArr[fd].ptrfiletable->writeoffset) = size;
        }

        else if (from == START)
        {
            if (size > MAXFILESIZE)
            {
                return -1;
            }

            if (size < 0)
            {
                return -1;
            }

            if (size > (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            }
            (UFTDArr[fd].ptrfiletable->writeoffset) = size;
        }

        else if (from == END)
        {
            if (((UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) + size) > MAXFILESIZE)
            {
                return -1;
            }

            if (((UFTDArr[fd].ptrfiletable->writeoffset) + size) < 0)
            {
                return -1;
            }

            (UFTDArr[fd].ptrfiletable->writeoffset) = ((UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize) + size);
        }

    }
}


void ls_file()
{
    int i = 0;
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("ERROR : There are no files \n");
        return;
    }

    printf("\n File Name \t Inode Number \t File Size \t Link Count \n ");
    printf("----------------------------------------------------------\n");
    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->inodeNumber, temp->FileActualSize, temp->LinkCount,temp->stage);
        }

        temp = temp->next;

    }

    printf("----------------------------------------------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if (fd < 0)
    {
        return -1;
    }

    if ((UFTDArr[fd].ptrfiletable) == NULL)
    {
        return -2;
    }

    temp = UFTDArr[fd].ptrfiletable->ptrinode;

    printf("----------------Statistical  Information about the file---------------------\n");
    printf("File Name %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->inodeNumber);
    printf("File size %d\n", temp->FileSize);
    printf("Actual File size %d\n", temp->FileActualSize);
    printf("Link Count %d\n", temp->LinkCount);
    printf("File size %d\n", temp->FileSize);
    printf("Reference Count %d\n", temp->ReferenceCount);
    printf("Stage of the File in Amit's Version Control System %d\n", temp->stage);

    if ((temp->permission) == 1)
    {
        printf("File Permisssion if Read Only\n");
    }


    else if ((temp->permission) == 2)
    {
        printf("File Permisssion if Write Only\n");
    }

    else if ((temp->permission) == 3)
    {
        printf("File Permisssion if Read & Write \n");
    }

    return 0;
}


int stat_file(char* name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
    {
        return -1;
    }

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;
    }

    if (temp == NULL)
    {
        return -2;
    }

    printf("----------------Statistical  Information about the file---------------------\n");
    printf("File Name %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->inodeNumber);
    printf("File size %d\n", temp->FileSize);
    printf("Actual File size %d\n", temp->FileActualSize);
    printf("Link Count %d\n", temp->LinkCount);
    printf("File size %d\n", temp->FileSize);
    printf("Reference Count %d\n", temp->ReferenceCount);
    printf("Stage of the File in Amit's Version Control System %d\n", temp->stage);

    if ((temp->permission) == 1)
    {
        printf("File Permisssion if Read Only\n");
    }


    else if ((temp->permission) == 2)
    {
        printf("File Permisssion if Write Only\n");
    }

    else if ((temp->permission) == 3)
    {
        printf("File Permisssion if Read & Write \n");
    }

    return 0;
}

int truncate_File(char* name)
{
    int fd = GetFDFromName(name);
    if (fd == 1)
    {
        return -1;
    }

    memset(UFTDArr[fd].ptrfiletable->ptrinode->Buffer, 0, 1024);
    UFTDArr[fd].ptrfiletable->ptrinode->stage = MODIFIED;
    UFTDArr[fd].ptrfiletable->readoffset = 0;
    UFTDArr[fd].ptrfiletable->writeoffset = 0;
    UFTDArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
    printf("\n This repository's status is : modified  by Amit's Version Control System \n");
    printf("\n");


}


//Version Control System

//Init Repository = git init
int init(int repository)
{
    printf("\n Initialised the Repository....\n");
    printf("\n");
    return(repository);
}

//get status = git status
void get_status()
{
    int i = 0;
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("ERROR : There are no files \n");
        return;
    }

    printf("\n File Name \t Inode Number \t File Size \t Link Count \n ");
    printf("----------------------------------------------------------\n");
    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->inodeNumber, temp->FileActualSize, temp->LinkCount);
        }

        temp = temp->next;

    }

    printf("-------------------------------------------------\n");
}

//Add = git add <name>
int AddFile(char* name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if ((name == NULL) || (mode <= 0))
    {
        return -1;
    }

    temp = Get_Inode(name);

    if (temp == NULL)
    {
        return -2;
    }

    if (temp->permission < mode)
    {
        return -3;
    }

    while (i < 50)
    {
        if (UFTDArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFTDArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if (UFTDArr[i].ptrfiletable == NULL)
    {
        return -1;
    }

    UFTDArr[i].ptrfiletable->count = 1;
    UFTDArr[i].ptrfiletable->mode = mode;
    if (mode == READ + WRITE)
    {
        UFTDArr[i].ptrfiletable->readoffset = 0;
        UFTDArr[i].ptrfiletable->writeoffset = 0;
    }

    else if (mode == READ)
    {
        UFTDArr[i].ptrfiletable->readoffset = 0;
    }

    else if (mode == WRITE)
    {
        UFTDArr[i].ptrfiletable->writeoffset = 0;
    }

    UFTDArr[i].ptrfiletable->ptrinode = temp;
    (UFTDArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

}

/////////////////////////////////////////////////////////////////////////////////
bool StageLog(int fd)
{
    //int month[]= {"JAN","FEB","MAR","APR","MAY","JUNE","JULY","AUG","SEPT","OCT","NOV","DEC"};
    //char month[] = { 'JAN','FEB','MAR','APR','MAY','JUN','JUL','AUG','SEPT','OCT','NOV','DEC' };
    char month[] = { 'JAN','FEB','MAR','APR','MAY','JUN','JUL','AUG','SEPT','OCT','NOV','DEC' };
    char FileName[50], arr[512];

    int ret = 0,count = 0;
    SYSTEMTIME lt;
    //Logfile fobj;
    //FILE* fp;
    GetLocalTime(&lt);
    UFTDArr[fd].ptrfiletable->ptrinode->hr = lt.wHour;
    UFTDArr[fd].ptrfiletable->ptrinode->month= lt.wMinute;
    UFTDArr[fd].ptrfiletable->ptrinode->date = lt.wDay;
    UFTDArr[fd].ptrfiletable->ptrinode->date = month[lt.wMonth - 1];

  
    std::cout << "Log file succesfully gets created as : " << UFTDArr[fd].ptrfiletable->ptrinode->FileName << "\n";
    std::cout << "Time of log file creation is->" << lt.wHour << ":" << lt.wMinute << " : " << lt.wDay << "th " << month[lt.wMonth - 1] << "\n";
    

    return(TRUE);
}




bool ReadLog()
{
    
    int i = 0;
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("ERROR : There are no files \n");
        return false;
    }

    printf("\n File Name \t Inode Number \t File Size \t Link Count \n ");
    printf("----------------------------------------------------------\n");
    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->inodeNumber, temp->FileActualSize, temp->LinkCount, temp->stage);
            std::cout <<"Hour : "<< temp->hr <<"\n" << "Minutes : " << temp->min <<"\n" << "Day : " << temp->date <<"\n" << "Month : "<< temp->month << "\n";
            std::cout << "\n";
        }

        temp = temp->next;

    }

    printf("----------------------------------------------------------------------------------------------\n");
    
    std::cout << "\n This is the log of the Amit's Version Control System \n" << "\n";
    std::cout << "\n";

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void AddIgnore(char* name)
{
    CreateFile(name, 3);
    printf("Ignore File Successfully Created.");

}



int main()
{
    char* ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[1024];

    //init repo
    int repository = 0;

    InitialiseSuperBlock();
    CreateDILB();

    while (1)
    {
        fflush(stdin);
        strcpy(str, "");

        printf("\nAmit VCS : >");
        fgets(str, 80, stdin);

        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if (count == 1)
        {
            if (strcmp(command[0], "ls") == 0)
            {
                ls_file();
            }

            else if (strcmp(command[0], "init") == 0)
            {
                repository = 1;
                init(repository);
            }

            else if (strcmp(command[0], ".ignore") == 0)
            {
                AddIgnore(command[0]);
                printf("\n");
            }

            else if (strcmp(command[0], "log") == 0)
            {
                bool out=ReadLog();
                if (out == true)
                {
                    std::cout << "\n" << "Successfully Read the log" << "\n";
                }
                else
                {
                    std::cout << "Log file could not be read" << "\n";
                }
                printf("\n");
            }



            else if (strcmp(command[0], "status") == 0)
            {
                if (repository == 1)
                {
                    get_status();
                    printf("\n");
                }
                else
                {
                    printf("You cannot create a repository. First Initialise the repository");
                    printf("\n");
                }
                
            }

            else if (strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("All files closed successfully\n");
                continue;
            }

            else if (strcmp(command[0], "clear") == 0)
            {
                system("cls");
                continue;
            }

            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }

            else if (strcmp(command[0], "exit") == 0)
            {
                printf("Terminating the Amit's Version Control System\n");
                break;
            }

            else
            {
                printf("\n error : command not found!!!\n");
                continue;
            }

        }
        if (repository == 1)
        {
            if (count == 2)
            {
                if (strcmp(command[0], "stat") == 0)
                {
                    ret = stat_file(command[1]);
                    if (ret == -1)
                        printf("Error : Incorrect parameters\n");
                    if (ret == -2)
                        printf("Error : There is no such file\n");
                    continue;
                }

                else if (strcmp(command[0], "close") == 0)
                {
                    ret = CloseFileByName(command[1]);
                    if (ret == -1)
                    {
                        printf("Error : there is no such file\n");
                    }
                    continue;
                }

                else if (strcmp(command[0], "rm") == 0)
                {
                    ret = rm_File(command[1]);
                    if (ret == -1)
                        printf("Error : There is no such file\n");
                    continue;
                }

                else if (strcmp(command[0], "man") == 0)
                {
                    man(command[1]);
                }

                else if (strcmp(command[0], "write") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if (fd == -1)
                    {
                        printf("Error : Incorrect parameters\n");
                        continue;
                    }
                    printf("Enter the data : \n");
                    scanf("%[^\n]s", arr);

                    ret = strlen(arr);
                    if (ret == 0)
                    {
                        printf("Error : Incorrect parameter\n");
                        continue;
                    }

                    ret = WriteFile(fd, arr, ret);

                    if (ret == -1)
                        printf("Error : Permission denied\n");

                    if (ret == -2)
                        printf("Error : There is no sufficient memory to write\n");

                    if (ret == -3)
                        printf("Error : It is not regular file\n");
                }

                else if (strcmp(command[0], "truncate") == 0)
                {
                    ret = truncate_File(command[1]);
                    if (ret == -1)
                        printf("Error : Incorrect parameter\n");
                }
                else
                {
                    printf("\n Error : Command not found!!!\n");
                }

            }

            else if (count == 3)
            {
                if (strcmp(command[0], "create") == 0)
                {
                    printf("in");
                    ret = CreateFile(command[1], atoi(command[2]));
                    if (ret >= 0)
                    {
                        printf("File is successfully created with file descriptor : %d \n", ret);
                        StageLog(ret);
                    }
                        
                    if (ret == -1)
                        printf("Error : Incorect parameters\n");
                    if (ret == -2)
                        printf("Error : There is no inodes\n");
                    if (ret == -3)
                        printf("Error : File already exists\n");
                    if (ret == -4)
                        printf("Error : Memory allocation failure\n");
                    continue;
                }


                else if (strcmp(command[0], "open") == 0)
                {
                    ret = OpenFile(command[1], atoi(command[2]));
                    if (ret >= 0)
                        printf("File is successfully opened with file descriptor : %d \n", ret);
                    if (ret == -1)
                        printf("Error : Incorrect parameters\n");
                    if (ret == -2)
                        printf("Error : File not present\n");
                    if (ret == -3)
                        printf("Error : Permission Denied\n");
                    continue;
                }

                else if (strcmp(command[0], "add") == 0)
                {
                    ret = OpenFile(command[1], atoi(command[2]));
                    if (ret >= 0)
                        printf("File is successfully opened with file descriptor : %d \n", ret);
                    if (ret == -1)
                        printf("Error : Incorrect parameters\n");
                    if (ret == -2)
                        printf("Error : File not present\n");
                    if (ret == -3)
                        printf("Error : Permission Denied\n");
                    continue;
                }

                else if (strcmp(command[0], "read") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if (fd == -1)
                    {
                        printf("Error : Incorrect parameter\n");
                        continue;
                    }
                    ptr = (char*)malloc(sizeof(atoi(command[2])) + 1);

                    if (ptr == NULL)
                    {
                        printf("Error : Memory Allocation failure\n");
                        continue;
                    }
                    ret = ReadFile(fd, ptr, atoi(command[2]));
                    if (ret == -1)
                        printf("Error : File Not existing\n");
                    if (ret == -2)
                        printf("Error : Permission Denied\n");
                    if (ret == -3)
                        printf("Error : Reached at end of file\n");
                    if (ret == -4)
                        printf("Error : It is not regular file\n");
                    if (ret == 0)
                        printf("Error : File empty\n");
                    if (ret > 0)
                    {
                        write(2, ptr, ret);
                    }
                    continue;

                }
                else
                {
                    printf("\nError : Command not found!!!\n");
                    continue;
                }
            }

            else if (count == 4)
            {
                if (strcmp(command[0], "lseek") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if (fd == -1)
                    {
                        printf("Error : Incorrect parameter\n");
                        continue;
                    }
                    ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
                    if (ret == -1)
                    {
                        printf("Unable to perform lseek\n");
                    }
                }
                else
                {
                    printf("\nError : Command not found!!!\n");
                    continue;
                }
            }
        }

        else
        {
            printf("\n Error : Command not found!!!");
            printf("\n First Initialise as a repository");
            printf("\n");
            continue;
        }

    }
    return 0;
}
