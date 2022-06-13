#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "protocol.h"
#include "check_send_recv.h"
#include <pthread.h>
#define PORT 5500
#include "singLL.h"

#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define BUFFER_SZ 1000
#define NAME_LEN 64
int uid = 10;
int roomUid = 1;
// client structure
int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};
// chua check invalid move

// client structure
typedef struct
{
    elementtype userInfo;
    int sockfd;
    int uid;
    
} client_t;

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
    char state[NAME_LEN];
    game_t *game;
} room_t;

client_t *clients[MAX_CLIENTS];
room_t *rooms[MAX_ROOMS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "queueManager.h"

char *getCommand(char *msg)
{
    char command[20];
    int count = 0;
    for (int i = 0; msg[i] != '\0'; i++)
    {
        if (msg[i] == '|')
        {
            break;
        }
        command[i] = msg[i];
        count = i;
    }
    command[count] = '\0';
    return command;
}
// send resposne
int send_message(response res, int uid)
{   int flag =0;
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                if (write(clients[i]->sockfd, &res, sizeof(res)) < 0)
                {
                    printf("ERROR: write to descriptor failed\n");
                    flag=0;
                    break;
                }else flag=1;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    return flag;
}

int checkRecvMsg(char buffer[1000], int newSocket,client_t *cli)
{   
    int flag;
    int byte_sent, byte_recv;
    int retry;
    
    userLogin userInfo; /*User login information*/
    response res;       /*Response message*/
    char *p;
    p = strtok(buffer, "|");
    // printf("%s",p);
    if (strcmp(p, "SELECT_WORK") == 0)
    {
        // lua chon cong viec khi moi vao

        int check;
        p = strtok(NULL, "|");
        check = atoi(p); // lua chon cua client gui ve
        if (check == 1)
        {
            printf("\nREADY_LOGIN\n");
            res.status = 200;
            strcpy(res.message, "Login");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        else if (check == 2)
        {
            res.status = 200;
            strcpy(res.message, "SignUp");
            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        else if (check == 3)
        {
            retry = 0;
            res.status = 200;
            strcpy(res.message, "Exit");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
            return 0;
        }
        else if (check == 4)
        {
            res.status = 200;
            strcpy(res.message, "Exit");
            char us[100];
            strcpy(us, strtok(NULL, "|"));
            node *p;
            for (p = root; p != NULL; p = p->next)
            {
                if (strcmp(us, p->element.name) == 0)
                {
                    p->element.status = 0;
                }
            }
            traversingList(root);
            byte_sent = send(newSocket, &res, sizeof(res), 0);
            return 0;
        }
        return Check_Send(byte_sent);
    }

    if (strcmp(p, "LOGIN") == 0)
    {
        strcpy(userInfo.user, strtok(NULL, "|"));
        printf("userName received :%s\n", userInfo.user);
        strcpy(userInfo.pass, strtok(NULL, "|"));
        printf("password received :%s\n", userInfo.pass);
        int fl = 1;
        node *p;
        for (p = root; p != NULL; p = p->next)
        {
            if (strcmp(userInfo.user, p->element.name) == 0 && strcmp(userInfo.pass, p->element.pass) == 0)
            {
                if (p->element.status == 0)
                {
                    p->element.status = 1;
                    // strcpy(p->element.ip, ip);
                    cli->userInfo = ( p->element);
                    printf("%s", cli->userInfo.name);
                    fl = 0;
                    break;
                }
                else
                {
                    fl = -1;
                    break;
                }
            }
        }

        if (fl == 0)
        {
            traversingList(root);
            res.status = 200;
            strcpy(res.message, "Login Successful");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        else if (fl == -1)
        {
            res.status = 400;
            strcpy(res.message, "Already Login");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        else
        {
            res.status = 401;
            strcpy(res.message, "Login Failure");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        return Check_Send(byte_sent);
    }
    if (strcmp(p, "LOGOUT") == 0)
    {
        char us[100];
        strcpy(us, strtok(NULL, "|"));
        node *p;
        for (p = root; p != NULL; p = p->next)
        {
            if (strcmp(us, p->element.name) == 0)
            {
                p->element.status = 0;
              
            }
        }
        traversingList(root);
        res.status = 200;
        strcpy(res.message, "Start");

        byte_sent = send(newSocket, &res, sizeof(res), 0);
        return Check_Send(byte_sent);
    }
    if (strcmp(p, "SIGNUP") == 0)
    {
        strcpy(userInfo.user, strtok(NULL, "|"));
        printf("userName received :%s\n", userInfo.user);
        strcpy(userInfo.pass, strtok(NULL, "|"));
        printf("password received :%s\n", userInfo.pass);
        int fl = 1;
        node *p;
        for (p = root; p != NULL; p = p->next)
        {
            if (strcmp(userInfo.user, p->element.name) == 0)
            {
                fl = 0;
                break;
            }
        }

        if (fl == 0)
        {
            res.status = 400;
            strcpy(res.message, "Register Fail");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        else
        {
            elementtype ele;
            strcpy(ele.name, userInfo.user);
            strcpy(ele.pass, userInfo.pass);
            ele.elo = 0;
            ele.status = 0;
            // strcpy(ele.ip, ip);
            insertAtHead(ele);
            traversingList(root);
            res.status = 200;
            strcpy(res.message, "Register Successful");

            byte_sent = send(newSocket, &res, sizeof(res), 0);
        }
        return Check_Send(byte_sent);
    }
    if (strcmp(p, "START_GAME") == 0)
    {
        
    
        res.status = 200;
        strcpy(res.message, "START");
        byte_sent = send(newSocket, &res, sizeof(res), 0);
        
        return Check_Send(byte_sent);
    }
    if (strcmp(p, "start") == 0)
    {
        int startgame = 0;
        room_t *room_game;
        
        pthread_mutex_lock(&rooms_mutex);

        for (int j = 0; j < MAX_ROOMS; j++)
        {
            if (rooms[j])
            {
                if (rooms[j]->player1->uid == cli->uid)
                {
                    if (rooms[j]->player2 != 0)
                    {
                        startgame = 1;
                        room_game = rooms[j];
                        break;
                    }

                    res.status=400;
                    sprintf(res.message, "[SERVER] 2 players are required to start the game\n");
                    byte_sent=send_message(res, cli->uid);
                    break;
                }
                else if (rooms[j]->player2->uid == cli->uid)
                {
                    res.status=400;
                    sprintf(res.message, "[SERVER] only the owner of the room can start\n");
                    byte_sent=send_message(res, cli->uid);
                    break;
                }
            }
        }

        pthread_mutex_unlock(&rooms_mutex);

        if (startgame == 1)
        {
            room_game->game = (game_t *)malloc(sizeof(game_t));
            room_game->game->gameStatus = 1;
            room_game->game->round = 0;
            room_game->game->playerTurn = room_game->player1->uid;
            strcpy(room_game->state, "playing now");

            for (int linha = 0; linha < 3; linha++)
            {
                for (int coluna = 0; coluna < 3; coluna++)
                {
                    room_game->game->board[linha][coluna] = '-';
                }
            }

            sleep(1);

            res.status=200;
            sprintf(res.message, "start game\n");
            byte_sent = send_message(res, room_game->player1->uid);

            sleep(0.1);

            res.status = 200;
            sprintf(res.message, "start game2\n");
            byte_sent = send_message(res, room_game->player2->uid);

            sleep(1);

            res.status = 200;
            bzero(res.message, BUFFER_SZ);
            sprintf(res.message, "%s\n", room_game->player2->userInfo.name);
            byte_sent = send_message(res, room_game->player1->uid);

            sleep(0.1);

            res.status = 200;
            bzero(res.message, BUFFER_SZ);
            sprintf(res.message, "%s\n", room_game->player1->userInfo.name);
            byte_sent = send_message(res, room_game->player2->uid);

            sleep(1);

            res.status = 200;
            bzero(res.message, BUFFER_SZ);
            sprintf(res.message, "vez1\n");
            byte_sent = send_message(res, room_game->player1->uid);

            sleep(0.2);

            res.status = 200;
            bzero(res.message, BUFFER_SZ);
            sprintf(res.message, "vez2\n");
            byte_sent=send_message(res, room_game->player2->uid);
        }
        return byte_sent;
    }
        if (strcmp(p, "list") == 0)
        {
            strcpy(res.message, "");
            pthread_mutex_lock(&rooms_mutex);

            for (int i = 0; i < MAX_ROOMS; i++)
            {
                if (rooms[i])
                {
                    char *list = (char *)malloc(BUFFER_SZ * sizeof(char));

                    if (rooms[i]->player2 != 0)
                    {
                        sprintf(list, "%i)\n    room state: %s\n    player1: %s\n    player2: %s\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->userInfo.name, rooms[i]->player2->userInfo.name);
                    }
                    else
                    {
                        sprintf(list, "%i)\n    room state: %s\n    player1: %s\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->userInfo.name);
                    }

                    strcat(res.message, list);

                    free(list);
                }
            }

            pthread_mutex_unlock(&rooms_mutex);
            res.status = 200;
            byte_sent = send_message(res, cli->uid);
            return byte_sent;
        }

        if (strcmp(p, "create") == 0)
        {
            flag = 0;

            pthread_mutex_lock(&rooms_mutex);

            for (int i = 0; i < MAX_ROOMS; i++)
            {
                if (rooms[i])
                {
                    if (rooms[i]->player1->uid == cli->uid)
                    {
                        res.status = 400;
                        sprintf(res.message, "[SERVER] you are already in the room\n");
                        byte_sent = send_message(res, cli->uid);
                        flag = 1;
                        break;
                    }

                    if (rooms[i]->player2 != 0)
                    {
                        if (rooms[i]->player2->uid == cli->uid)
                        {
                            res.status = 400;
                            sprintf(res.message, "[SERVER] you are already in the room\n");
                            byte_sent = send_message(res, cli->uid);
                            flag = 1;
                            break;
                        }
                    }
                }
            }

            pthread_mutex_unlock(&rooms_mutex);

            if (flag != 1)
            {
                // clients settings
                room_t *room = (room_t *)malloc(sizeof(room_t));
                room->player1 = cli;
                room->player2 = 0;
                room->uid = roomUid;
                strcpy(room->state, "waiting for second player");

                // add room to queue
                queue_add_room(room);
                res.status = 200;
                sprintf(res.message, "[SERVER] you created a new room number %i\n", roomUid);
                roomUid++;
                byte_sent = send_message(res, cli->uid);
            }
            return byte_sent;
        }
        if (strcmp(p, "join") == 0)
        {
            char roomNumber[10];
            strcpy(roomNumber, strtok(NULL, "|"));
            int researched = 0;
            int already = 0;
            pthread_mutex_lock(&rooms_mutex);

            for (int j = 0; j < MAX_ROOMS; j++)
            {
                if (rooms[j])
                {
                    if (rooms[j]->player1->uid == cli->uid)
                    {
                        already = 1;

                        res.status = 400;
                        sprintf(res.message, "[SERVER] you are already in the room number: %i\n", rooms[j]->uid);
                        byte_sent = send_message(res, cli->uid);
                        break;
                    }
                }
            }

            pthread_mutex_unlock(&rooms_mutex);

            if (already == 1)
            {
                return byte_sent;
            }

            pthread_mutex_lock(&rooms_mutex);

            for (int i = 0; i < MAX_ROOMS; i++)
            {
                if (rooms[i])
                {
                    if (rooms[i]->uid == atoi(roomNumber))
                    {
                        researched = 1;

                        if (rooms[i]->player2 != 0)
                        {
                            if (rooms[i]->player2->uid == cli->uid)
                            {
                                res.status = 400;
                                strcpy(res.message, "[SERVER] you are already in the room\n");
                                byte_sent = send_message(res, cli->uid);
                                break;
                            }

                            res.status = 400;
                            sprintf(res.message, "[SERVER] room number: %i, is already full\n", rooms[i]->uid);
                            byte_sent = send_message(res, cli->uid);
                            break;
                        }

                        rooms[i]->player2 = cli;
                        strcpy(rooms[i]->state, "waiting start");
                        res.status = 200;
                        printf("%s enter the room number: %s\n", cli->userInfo.name, roomNumber);
                        sprintf(res.message, "[SERVER] '%s' entered your room\n", cli->userInfo.name);
                        byte_sent = send_message(res, rooms[i]->player1->uid);

                        res.status = 200;
                        sprintf(res.message, "[SERVER] you has entered the room number: %s\n", roomNumber);
                        byte_sent = send_message(res, cli->uid);
                        break;
                    }
                }
            }

            pthread_mutex_unlock(&rooms_mutex);

            if (researched == 0)
            {
                res.status = 200;
                sprintf(res.message, "[SERVER] could not find the room number %s\n", roomNumber);
                byte_sent = send_message(res, cli->uid);
            }
            return byte_sent;
        }
        if (strcmp(p, "leave") == 0)
        {
            int remove_room = 0;
            int room_number = 0;

            pthread_mutex_lock(&rooms_mutex);

            for (int i = 0; i < MAX_ROOMS; i++)
            {
                if (rooms[i])
                {
                    if (rooms[i]->player1->uid == cli->uid)
                    {
                        if (rooms[i]->player2 != NULL)
                        {
                            res.status = 200;
                            sprintf(res.message, "[SERVER] %s left the room, now you are the owner\n", rooms[i]->player1->userInfo.name);
                            byte_sent = send_message(res, rooms[i]->player2->uid);

                            rooms[i]->player1 = rooms[i]->player2;
                            rooms[i]->player2 = NULL;
                            strcpy(rooms[i]->state, "waiting for secound player");
                        }
                        else
                        {
                            remove_room = 1;
                            room_number = rooms[i]->uid;
                        }

                        res.status = 200;
                        sprintf(res.message, "[SERVER] you left the room %i\n", rooms[i]->uid);
                        byte_sent = send_message(res, cli->uid);
                        break;
                    }
                    else if (rooms[i]->player2->uid == cli->uid)
                    {
                        res.status = 200;
                        sprintf(res.message, "[SERVER] %s left the room\n", rooms[i]->player1->userInfo.name);
                        byte_sent = send_message(res, rooms[i]->player1->uid);

                        rooms[i]->player2 = 0;
                        strcpy(rooms[i]->state, "waiting for secound player");

                        res.status = 200;
                        sprintf(res.message, "[SERVER] you left the room %i\n", rooms[i]->uid);
                        byte_sent = send_message(res, cli->uid);
                        break;
                    }
                }
            }

            pthread_mutex_unlock(&rooms_mutex);

            if (remove_room == 1)
            {
                queue_remove_room(room_number);
                roomUid--;
            }
            return byte_sent;
        }
        // wait for other to log in to start a game
        return 0;
    }

void *serverthread(void *arg)
{
    

    char name[NAME_LEN];
    int number;
    int leave_flag = 0;
    int flag = 0;
    client_t *cli = (client_t *)arg;

    elementtype userInfo;
    int check_status = 1;
    char buffer[1024];
    int newSocket = cli->sockfd; /*Thread Socket descriptor*/
    printf("Thread ID:%d\n", newSocket);
    traversingList(root);

    response res; /*Response message*/
    res.status = 200;
    strcpy(res.message, "Start");
    int byte_sent = send(newSocket, &res, sizeof(res), 0);
    if (byte_sent <= 0)
    {
        printf("Fail to receive");
        close(newSocket);
        pthread_exit(0);
    }
    while (1)
    {
        int byte_recv = recv(newSocket, buffer, 1024, 0);
        if (byte_recv <= 0)
        {
            printf("Fail to receive");

            break;
        }
        buffer[byte_recv] = '\0';
        printf("CLient:%s\n", buffer);

        if (checkRecvMsg(buffer, newSocket,cli) <= 0)
        {
            break;
        }
    }
    printf("Close thread:%d\n", newSocket);
    close(newSocket);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    importTextFile("database.txt");
    int sockfd, ret;
    struct protoent *ptrp;
    struct sockaddr_in serverAddr;
    int newSocket;
    struct sockaddr_in newAddr;
    response res;
    socklen_t addr_size;
    short SERV_PORT;
    char buffer[1024];
    pthread_t tid;
    int byte_recv, byte_sent;

    if (argc != 2)
    {
        printf("Usage: %s <TCP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    SERV_PORT = atoi(argv[1]);

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERV_PORT);
    if (((ptrp = getprotobyname("tcp"))) == 0)
    {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }

    sockfd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sockfd < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", SERV_PORT);

    if (listen(sockfd, 10) == 0)
    {
        printf("[+]Listening....\n");
    }
    else
    {
        printf("[-]Error in binding.\n");
    }

    while (1)
    { // mark
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        printf("New connection\n");
        if (newSocket < 0)
        {
            exit(1);
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        
        cli->sockfd = newSocket;
        cli->uid = uid++;

        // add client to queue
        queue_add_client(cli);
        pthread_create(&tid, NULL, serverthread, (void *)cli);
    }
    printf("End server");
    close(sockfd);

    return 0;
}