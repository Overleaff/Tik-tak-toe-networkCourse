#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef struct phoneaddress_t
{
    char name[50];
    char pass[50];
    int elo;
    // char ip[50];
    int status; // 0:offline
} elementtype;

struct t
{
    elementtype element;
    struct t *next;
};
/*rewrite read userNode diplay userNode*/
typedef struct t userNode;
userNode *root2 = NULL;
userNode *prev2, *cur2;

userNode *makeNewNode2(elementtype element)
{
    userNode *new = (userNode *)malloc(sizeof(userNode));
    new->element = element;
    new->next = NULL;
    return new;
}

elementtype readNode2()
{
    elementtype tmp;
    printf("Input name:");
    scanf("%s", tmp.name);
    printf("Input pass:");
    scanf("%s", tmp.pass);
    printf("Input elo:");
    scanf("%d", &tmp.elo);

    return tmp;
}

void displayNode2(userNode *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    elementtype tmp = p->element;
    // printf("%-20s\t%-17s\t%-25d%-25s%-25d%-p\n", tmp.name, tmp.pass, tmp.elo, tmp.ip,tmp.status, p->next);
    printf("%-20s\t%-17s\t%-25d%-25d%-p\n", tmp.name, tmp.pass, tmp.elo, tmp.status, p->next);
}

void insertAtHead2(elementtype ele)
{
    userNode *new = makeNewNode2(ele);
    new->next = root2;
    root2 = new;
    cur2 = root2;
}

void insertAftercur2rent(elementtype element)
{
    userNode *new = makeNewNode2(element);
    if (root2 == NULL)
    {
        root2 = new;
        cur2 = root2;
    }
    else if (cur2 == NULL)
    {
        printf("cur2rent pointer is nULL.\n");
        return;
    }
    else
    {
        new->next = cur2->next;
        cur2->next = new;
        prev2 = cur2;
        cur2 = cur2->next;
    }
}

void insertBeforecur2rent(elementtype e)
{
    userNode *new = makeNewNode2(e);
    userNode *tmp = root2;

    while (tmp != NULL && tmp->next != cur2 && cur2 != NULL)
    {
        tmp = tmp->next;
        prev2 = tmp;
    }

    if (root2 == NULL)
    {
        root2 = new;
        cur2 = root2;
        prev2 = NULL;
    }
    else
    {
        new->next = cur2;
        if (cur2 == root2)
        {
            root2 = new;
        }
        else
            prev2->next = new;
        cur2 = new;
    }
}

void freeList2(userNode *root2)
{
    userNode *to_free = root2;
    while (to_free != NULL)
    {
        root2 = root2->next;
        free(to_free);
        to_free = root2;
    }
}

void deletefirstElement2()
{
    userNode *del = root2;
    if (del == NULL)
        return;
    root2 = del->next;
    free(del);
    cur2 = root2;
    prev2 = NULL;
}

void deletecur2()
{
    if (cur2 == NULL)
        return;
    if (cur2 == root2)
        deletefirstElement2();
    else
    {
        prev2->next = cur2->next;
        free(cur2);
        cur2 = prev2->next;
    }
}

userNode *list_reverse2(userNode *root2)
{
    userNode *cur2, *prev2;
    cur2 = prev2 = NULL;
    while (root2 != NULL)
    {
        cur2 = root2;
        root2 = root2->next;
        cur2->next = prev2;
        prev2 = cur2;
    }
    return prev2;
}

void deleteAtposi2(int pos)
{
    cur2 = root2;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur2->next != NULL)
        {
            prev2 = cur2;
            cur2 = cur2->next;
        }
        else
            break;
    }
    if (i < pos || pos < 0)
    {
        printf("No userNode at postion.\n");
        return;
    }
    deletecur2();
}

void insertAtposi2(elementtype ele, int pos)
{
    int i;
    cur2 = root2;
    if (pos <= 0)
    {
        insertBeforecur2rent(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur2->next) != NULL)
            cur2 = cur2->next;
        insertAftercur2rent(ele);
    }
}

void traversingList2(userNode *root2)
{
    userNode *p;
    for (p = root2; p != NULL; p = p->next)
        displayNode2(p);
}

void importTextFile(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    elementtype tmp;
    int i = 0;
    freeList2(root2);
    while (fscanf(fp, "%s %s %d %d", tmp.name, tmp.pass, &tmp.elo, &tmp.status) != EOF)
    {
        insertAtHead2(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

// Found the name at line
int getLine2(char *filename, char *name)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
    }
    elementtype tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d %d", tmp.name, tmp.pass, &tmp.elo, &tmp.status) != EOF)
    {
        line = line + 1;
        if (strcmp(name, tmp.name) == 0)
        {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (found == 1)
    {
        printf("Found %s at line %d \n", name, line);
        return line;
    }
    else
        printf("Found at line 0\n");
        return 0;

   
}

// save name user withdata
void saveData1(elementtype ele){
    // update file path
    int line = getLine2("database.txt",ele.name);
    FILE *fPtr;
    FILE *fTemp;
    char path[100];

    char buffer[1000];
    char newline[1000];
    int count;

    /* Remove extra new line character from stdin */
    fflush(stdin);

    printf("Replace '%d' line with: ", line);

    char tmp1[50];

    strcat(newline, ele.name);
    strcat(newline, " ");
    strcat(newline, ele.pass);
    strcat(newline, " ");
    sprintf(tmp1, "%d", ele.elo);
    strcat(newline, tmp1);
    strcat(newline, " ");
    sprintf(tmp1, "%d", ele.status);
    strcat(newline, tmp1);
    strcat(newline, "\n");

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
    while ((fgets(buffer, 1000, fPtr)) != NULL)
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
}

// skip line to update data
// return ip of player to challegne
/*
char * getNumberActive(char *yourName){
    userNode *p;
    int count=0;
    for(p=root2;p!=NULL;p=p->next){
      if(strcmp(p->element.name,yourName)!=0 && p->element.status==1){
          return p->element.ip;
      }
    }
    return NULL;
} */
void append(char *str)
{
    // append to last line
    FILE *fptr3;
    int i, n;

    char fname[20];

    printf("\n\n Append multiple lines at the end of a text file :\n");
    printf("------------------------------------------------------\n");
    printf(" Input the file name to be opened : ");

    fptr3 = fopen("database.txt", "a");
    printf(" Input the number of lines to be written : ");
    // scanf("%d", &n);
    printf(" The lines are : \n");

    fprintf(fptr3, "\n%s", str);

    fclose(fptr3);
}

void saveData(char *filename, elementtype user)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
      
    }
    elementtype tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d", tmp.name, tmp.pass, &tmp.elo) != EOF)
    {
        line = line + 1;
        if (strcmp(user.name, tmp.name) == 0)
        {
            // fprintf(fp, "%s %s %d", tmp.name, tmp.pass, tmp.elo);
            fprintf(fp, "tuan %s %d", tmp.pass, tmp.elo);
            break;
        }
    }
    fclose(fp);
}