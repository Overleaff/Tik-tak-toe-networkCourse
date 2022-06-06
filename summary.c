#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
typedef struct phonedb
{
    char model[20];
    char memory;
    char size;
    char price;

} phoneaddress;

struct node_t
{
    phoneaddress element;
    struct node_t *next;
};

typedef struct node_t node;
node *root, *cur;
node *prev;

node *makeNewNode(phoneaddress element)
{
    node *new = (node *)malloc(sizeof(node));
    new->element = element;
    new->next = NULL;
    return new;
}

phoneaddress readNode()
{
    phoneaddress tmp;
    printf("Input model:");
    scanf("%s", tmp.model);
    printf("Input memory:\n");
    scanf("%s", tmp.memory);
    printf("Input size:\n");
    scanf("%s", tmp.size);
    printf("Input price:\n");
    scanf("%s", tmp.price);
    return tmp;
}

void displayNode(node *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    phoneaddress tmp = p->element;
    printf("%-20s\t%-17s\t%-25s\t%s%-p\n", tmp.model, tmp.memory, tmp.size, tmp.price, p->next);
}

void insertAtHead(phoneaddress element)
{
    node *new = makeNewNode(element);
    new->next = root;
    root = new;
    cur = root;
}

void traversingList(node *root)
{
    node *p;
    for (p = root; p != NULL; p = p->next)
        displayNode(p);
}

void insertAfterCurrent(phoneaddress element)
{
    node *new = makeNewNode(element);
    if (root == NULL)
    {
        root = new;
        cur = root;
    }
    else if (cur == NULL)
    {
        printf("Current pointer is nULL.\n");
        return;
    }
    else
    {
        new->next = cur->next;
        cur->next = new;
        prev = cur;
        cur = cur->next;
    }
}

void insertBeforeCurrent(phoneaddress e)
{
    node *new = makeNewNode(e);
    if (root == NULL)
    {
        root = new;
        cur = root;
        prev = NULL;
    }
    else
    {
        new->next = cur;
        if (cur == root)
        {
            root = new;
        }
        else
            prev->next = new;
        cur = new;
    }
}

void freeList(node *root)
{
    node *to_free = root;
    while (to_free != NULL)
    {
        root = root->next;
        free(to_free);
        to_free = root;
    }
}

void deletefirstelement()
{
    node *del = root;
    if (del == NULL)
        return;
    root = del->next;
    free(del);
    cur = root;
    prev = NULL;
}

void deletecur()
{
    if (cur == NULL)
        return;
    if (cur == root)
        deletefirstelement();
    else
    {
        prev->next = cur->next;
        free(cur);
        cur = root;
    }
}

node *list_reverse(node *root)
{
    node *cur, *prev;
    cur = prev = NULL;
    while (root != NULL)
    {
        cur = root;
        root = root->next;
        cur->next = prev;
        prev = cur;
    }
    return prev;
}

void deleteAtposi(int pos)
{
    cur = root;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur->next != NULL)
        {
            prev = cur;
            cur = cur->next;
        }
        else
            break;
    }
    if (i < pos || pos < 0)
    {
        printf("No node at postion.\n");
        return;
    }
    deletecur();
}

void insertAtposi(phoneaddress e, int pos)
{
    int i;
    cur = root;
    if (pos <= 0)
    {
        insertBeforeCurrent(e);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur->next) != NULL)
            cur = cur->next;
        insertAfterCurrent(e);
    }
}

node *gotoposition(node *root, int n)
{
    node *go;
    go = root;
    int i = 0;
    if (go == NULL || n < 0)
    {
        return NULL;
    }
    while (go->next != NULL)
    {
        if (i == n)
            break;
        else
        {
            i = i + 1;
            go = go->next;
        }
        if (i < n)
        {
            printf("The max position is %d.\n", i);
            return NULL;
        }
        return go;
    }
}

node *splitlist(int n1, int n2)
{
    node *root1 = NULL;
    if (n2 == 0)
    {
        printf("Cannot extract zero length list.\n");
        return root1;
    }
    prev = gotoposition(root, n1 - 1);
    root1 = gotoposition(root, n1);
    cur = gotoposition(root, n2 + n1);
    if (prev != NULL)
        prev->next = cur;
    else
        root = cur;
    cur = gotoposition(root1, n2 - 1);
    if (cur != NULL)
        cur->next = NULL;
    cur = root;
    prev = NULL;
    return root1;
}

int check(phoneaddress p, char *c)
{
    if (strcmp(p.model, c) == 0)
    {
        return 1;
    }
    else
        return 0;
}

void importTextFile(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    phoneaddress tmp;
    int i = 0;
    freeList(root);
    while (fscanf(fp, "%s %s %s %s", tmp.model, tmp.memory, tmp.size, tmp.price) != EOF)
    {
        insertAtHead(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

void importBinaryFile(char *filename)
{
    FILE *fp1;

    if ((fp1 = fopen(filename, "rb")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    phoneaddress temp;
    int i = 0, irc;
    if (root != NULL)
        free(root);
    while (1)
    {
        irc = fread(&temp, sizeof(phoneaddress), 1, fp1);
        if (irc = 0)
            break;
        insertAtHead(temp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp1);
}

phoneaddress adjustdata(phoneaddress e)
{
    phoneaddress p;
    strcpy(p.model, e.model);
    printf("Update memory:");
    scanf("%s", p.memory);
    printf("Update screen size:");
    scanf("%s", p.size);
    printf("Update price:");
    scanf("%s", p.price);
    return p;
}

void savebinaryfile(char *filename)
{
    FILE *fout;
    node *p;
    phoneaddress temp;
    int i = 0;
    if ((fout = fopen(filename, "rb")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    if (root = NULL)
    {
        printf("empty list.\n");
    }
    else
    {
        for (p = root; p->next != NULL; p = p->next)
        {
            temp = p->element;
            fwrite(&temp, sizeof(phoneaddress), 1, fout);
            i++;
        }
        printf("%d record write from list to file.\n", i);
    }
    fclose(fout);
}

void searchandupdate(char *mod)
{
    int flag = -1;
    node *p;
    if (root = NULL)
    {
        printf("Empty list.\n");
    }
    for (p = root; p->next != NULL; p = p->next)
    {
        if (strcmp(p->element.model, mod) == 0)
        {
            cur = p;
            printf("\n");
            displayNode(p);
            printf("Do you want to update? 1yes 2no");
            scanf("%d", &flag);
            if (flag == 1)
                p->element = adjustdata(p->element);
            break;
        }
    }
    if (flag == -1)
        printf("Model not found.\n");
}

int main()
{
    int i;
    node *root1 = NULL;
    int choice, pos, ins, n1, n2;
    char temp;
    char mod[15], file[15];
    phoneaddress ph;
    char filename[100];
    do
    {
        printf("1.IMport from PhoneDB.txt\n");
        printf("2.Import from dat\n");
        printf("3.Display\n");
        printf("4.Add new phone\n");
        printf("5.Insert at position\n");
        printf("6.Delete at position\n");
        printf("7.Delete current\n");
        printf("8.Delete first\n");
        printf("9.Search and update\n");
        printf("10.Divide an extract\n");
        printf("11.Reverse list\n");
        printf("12.Save to file\n");
        printf("13.Quit\n13.Import from dat\n");
        printf("Enter your choice:");
        scanf("%d", &i);
        switch (i)
        {
        case 1:
            importTextFile("phonedb.txt");
            break;
        case 3:
            printf("Model\tMEmory\tScreen size\tPrice\n");
            traversingList(root);
            break;
        case 4:
            printf("1.Insert before.\n");
            printf("2.Insert after.\n");
            printf("Enter your choice(1or2):");
            scanf("%s", &ins);
            printf("MoDEL:");
            scanf("%s", ph.model);
            printf("Memory:");
            scanf("%s", ph.memory);
            printf("Size:");
            scanf("%s", ph.size);
            printf("Price");
            scanf("%s", ph.price);
            if (ins == 1)
            {
                insertBeforeCurrent(ph);
            }
            else if (ins == 2)
            {
                insertAfterCurrent(ph);
            }
            break;
        case 5:
            printf("Insert at what position:");
            scanf("%d", &pos);
            printf("MoDEL:");
            scanf("%s", ph.model);
            printf("Memory:");
            scanf("%s", ph.memory);
            printf("Size:");
            scanf("%s", ph.size);
            printf("Price");
            scanf("%s", ph.price);
            insertAtposi(ph, pos);
            break;
        case 6:
            printf("Delete curent node.\n");
            deletecur();
            traversingList(root);
            break;
        case 7:
            printf("Delete first node.\n");
            deletefirstelement();
            traversingList(root);
            break;
        case 8:
            printf("What model you want to update for?");
            scanf("%s", mod);
            searchandupdate(mod);
            break;
        case 9:
            printf("Offset to extract:");
            scanf("%d", &n1);
            printf("Length of new list:");
            scanf("%d", &n2);
            root = splitlist(n1, n2);
            printf("List 1:\n");
            traversingList(root1);
            printf("list 2:\n");
            traversingList(root);
            break;
        case 10:
            printf("Reverse node.\n");
            root = list_reverse(root);
            traversingList(root);
            break;
        case 11:
            printf("Save to file:");
            scanf("%s", filename);
            savebinaryfile(filename);
        case 2:
            importBinaryFile("phonebook.dat");
            break;
            break;
        case 12:
            freeList(root);
            freeList(root1);
            break;
        default:
            break;
        }
    } while (1);

    return 0;
}
