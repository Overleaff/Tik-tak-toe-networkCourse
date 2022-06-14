#include <stdio.h>
#include <stdlib.h>
#include "singLL.h"
#include <string.h>
#include <ctype.h>
#define BUFFER_SIZE 1000
int main()
{ 

    //update file path
    int line = getLine("database.txt","tu");
    FILE *fPtr;
    FILE *fTemp;
    char path[100];

    char buffer[BUFFER_SIZE];
    char newline[BUFFER_SIZE];
    int  count;
    if(line ==0) return 0;
    

     

    /* Remove extra new line character from stdin */
    fflush(stdin);

    printf("Replace '%d' line with: ", line);
    fgets(newline, BUFFER_SIZE, stdin);

    /*  Open all required files */
    fPtr = fopen("database.txt", "r");
    fTemp = fopen("replace.tmp", "w");

    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL || fTemp == NULL)
    {
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        exit(EXIT_SUCCESS);
    }

    /*
     * Read line from source file and write to destination
     * file after replacing given line.
     */
    count = 0;
    while ((fgets(buffer, BUFFER_SIZE, fPtr)) != NULL)
    {
        count++;

        /* If current line is line to replace */
        if (count == line)
            fputs(newline, fTemp);
        else
            fputs(buffer, fTemp);
    }

    /* Close all files to release resource */
    fclose(fPtr);
    fclose(fTemp);

    /* Delete original source file */
    remove("database.txt");

    /* Rename temporary file as original file */
    rename("replace.tmp", "database.txt");

    printf("\nSuccessfully replaced '%d' line with '%s'.", line, newline);   
    
    // append to last line
    FILE *fptr3;
    int i, n;
    char str[100];
    char fname[20];
    char str1;

    printf("\n\n Append multiple lines at the end of a text file :\n");
    printf("------------------------------------------------------\n");
    printf(" Input the file name to be opened : ");
    
    fptr3 = fopen("database.txt", "a");
    printf(" Input the number of lines to be written : ");
    //scanf("%d", &n);
    printf(" The lines are : \n");
    
        fgets(str, sizeof(str), stdin);
        fprintf(fptr3, "\n%s", str);
            
        fclose(fptr3);

        // importTextFile("database.txt");

        // printf("%s", getNumberActive("tu"));
        /*
        for (i = 0; i < 4; i++)
        {
            tmp = readNode();
            insertAtHead(tmp);
            displayNode(root);
        };
        printf("Info\n");
        traversingList(root);

        printf("test for reverse list.\n");
        root = list_reverse(root);
        traversingList(root);

        printf("Delete first node.\n");  //position start from 0
        deletefirstelement();
        traversingList(root);

        printf("Delete curent node.\n");
        deletecur();
        traversingList(root);

        printf("Enter the position.\n");
        scanf("%d", &n);
        printf("Delete at position\n");
        deleteAtposi(n);
        traversingList(root);

        printf("Enter the position.\n"); // 1 insert after 1;
        scanf("%d", &i);
        printf("Print at position\n");
        tmp = readNode();
        insertAtposi(tmp, i);
        traversingList(root);
         */
        // freeList2(root2);
        return 0;
}
