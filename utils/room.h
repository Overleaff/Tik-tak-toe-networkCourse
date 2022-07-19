#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//add mutex when delete create join login
typedef struct
{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    elementtype userInfo;
    char username[100];   
   
} client_t;
// name khi chua login tmp
typedef struct
{
    char board[3][3];
    int gameStatus;
    int round;
    int playerTurn;
} game_t;

// room structure
typedef struct
{
    client_t *player1;
    client_t *player2;
    unsigned int uid;
    char state[100];
    game_t *game;
    char roomType[10];
} room_t;



struct node_t
{
    room_t element;
    struct node_t *next;
};
/*rewrite read node diplay node*/
typedef struct node_t node;
node *root = NULL;
node *prev, *cur;

node *makeNewNode(room_t element)
{
    node *new = (node *)malloc(sizeof(node));
    new->element = element;
    new->next = NULL;
    return new;
}
/*
room_t readNode()
{
    room_t tmp;
    printf("Input name:");
    scanf("%s", tmp.name);
    printf("Input pass:");
    scanf("%s", tmp.pass);
    printf("Input elo:");
    scanf("%d", &tmp.elo);

    return tmp;
}*/

void displayNode(node *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    room_t tmp = p->element;
    printf("Game room:%d\n",tmp.uid);
    /*
    printf("Round:%d\nPlayer Turn:%d", tmp.game->round, tmp.game->playerTurn);
    printf("Room\n");
    printf("Roomid:%d\nRoom type:%-20s\nState:%-100s\n", tmp.uid, tmp.roomType, tmp.state);
    printf("Player\n");*/
    printf("User 1:%s\nElo:%d\n",tmp.player1->userInfo.name,tmp.player1->userInfo.elo);
    //printf("User 2:%s\nElo:%d", tmp.player2->userInfo.name, tmp.player2->userInfo.elo);
}

void insertAtHead(room_t ele)
{
    pthread_mutex_lock(&rooms_mutex);
    node *new = makeNewNode(ele);
    new->next = root;
    root = new;
    cur = root;
    pthread_mutex_unlock(&rooms_mutex);
}

void insertAfterCurrent(room_t element)
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

void insertBeforeCurrent(room_t e)
{
    node *new = makeNewNode(e);
    node *tmp = root;

    while (tmp != NULL && tmp->next != cur && cur != NULL)
    {
        tmp = tmp->next;
        prev = tmp;
    }

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

void deletefirspassement()
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
        deletefirspassement();
    else
    {
        prev->next = cur->next;
        free(cur);
        cur = prev->next;
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

void deleteAtposi(int roomNumber)
{   printf("ROOM:%d\n",roomNumber);
    int pos = -1;
    node *p=root;
    int index = 0;
    int found = 0;
    pthread_mutex_lock(&rooms_mutex);
   
 
    // traverse till then end of the linked list
    while (p != NULL)
    {
        pos++;
        if (p->element.uid== roomNumber)
        {
            found=1;
            break;
        }
        p = p->next;
      
    }
    /*
    for(p=root;p!=NULL;p++){
        pos = pos + 1;
        if(p->element.uid == roomNumber){
            found = 1;
            break;
        }
    }*/
    if(found==0){
        pos = -1 ;
    }
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
   
    pthread_mutex_unlock(&rooms_mutex);
}

void insertAtposi(room_t ele, int pos)
{
    int i;
    cur = root;
    if (pos <= 0)
    {
        insertBeforeCurrent(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur->next) != NULL)
            cur = cur->next;
        insertAfterCurrent(ele);
    }
}

void traversingList(node *root)
{
    node *p;
    for (p = root; p != NULL; p = p->next)
        displayNode(p);
}
/*
void importTextFile(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    room_t tmp;
    int i = 0;
    freeList(root);
    while (fscanf(fp, "%s %s %d %s %d", tmp.name, tmp.pass, &tmp.elo, tmp.ip, &tmp.status) != EOF)
    {
        insertAtHead(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

// Found the name at line
void getLine(char *filename, char *name)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    room_t tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d %s %d", tmp.name, tmp.pass, &tmp.elo, tmp.ip, &tmp.status) != EOF)
    {
        line = line + 1;
        if (strcmp(name, tmp.name) == 0)
        {
            found = 1;
            break;
        }
    }
    if (found == 1)
    {
        printf("Found %s at line %d \n", name, line);
    }
    else
        printf("Found at line 0");
    fclose(fp);
}   */
