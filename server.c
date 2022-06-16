#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "singLL.h"
#include <pthread.h>
#include <sys/types.h>
#include "customSTD.h"

#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define BUFFER_SZ 2048
#define NAME_LEN 100

static _Atomic unsigned int cli_count = 0;
static int uid = 10;
static int roomUid = 1;

int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

// client structure
#include "room.h"

client_t *clients[MAX_CLIENTS];
room_t *rooms[MAX_ROOMS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "queueManager.h"

void send_message(char *message, int uid)
{
    response res;
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                // ass res.mess res.status
                //res.status = status;
                //strcpy(res.message, message);
                if (write(clients[i]->sockfd, message, strlen(message)) < 0)
                {
                    printf("ERROR: write to descriptor failed\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg)
{
    char buffer[BUFFER_SZ];
    char command[BUFFER_SZ];
    int number;
    char name[NAME_LEN];
    int leave_flag = 0;
    int flag = 0;
    int isRank = 0;
    int isLogin = 0; /* logout then isLogin=0 */
    client_t *cli = (client_t *)arg;
    char user[100];
    char pass[100];

    // name nhan tin hieu
        recv(cli->sockfd, name, NAME_LEN, 0);
  
        cli->elo = 1200;
        strcpy(cli->name, "new member");
        sprintf(buffer, "> %s has joined\n", cli->name);
        printf("%s", buffer);

        bzero(buffer, BUFFER_SZ);
        sprintf(buffer, "ok1");
        send_message(buffer, cli->uid);


    bzero(buffer, BUFFER_SZ);
    char *p;
    while (leave_flag == 0)
    {
        command[0] = '\x00';
        number = 0;

        int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);

        if (receive > 0)
        {
            if (strlen(buffer) > 0)
            {

                // send_message(buffer, cli->uid);
                trim_lf(buffer, strlen(buffer));
                printf("> client: '%s' has been send '%s' command\n", cli->name, buffer);
                sscanf(buffer, "%s %i", &command[0], &number);
                if (strstr(buffer, "GUEST"))
                {
                    p = strtok(buffer, "|");
                    strcpy(name, strtok(NULL, "|"));
                    strcpy(cli->name,name);
                    bzero(buffer, BUFFER_SZ);
                    sprintf(buffer, "ok");
                    send_message(buffer, cli->uid);
                }
                    if (strstr(buffer, "SIGNUP"))
                    { // TODO:luu vao file
                        p = strtok(buffer, "|");
                        strcpy(user, strtok(NULL, "|"));
                        printf("userName register received :%s\n", user);
                        strcpy(pass, strtok(NULL, "|"));
                        printf("password register received :%s\n", pass);
                        int fl = 1;
                        userNode *n;
                        for (n = root2; n != NULL; n = n->next)
                        {
                            if (strcmp(user, n->element.name) == 0)
                            {
                                fl = 0;
                                break;
                            }
                        }

                        if (fl == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "Name exist.Register fail.\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            elementtype ele;
                            strcpy(ele.name, user);
                            strcpy(ele.pass, pass);
                            ele.elo = 1200;
                            ele.status = 0;
                            char str[1000] = "";
                            char tmp[50];

                            strcat(str, user);
                            strcat(str, " ");
                            strcat(str, pass);
                            strcat(str, " ");
                            sprintf(tmp, "%d", ele.elo);
                            strcat(str, tmp);
                            strcat(str, " ");
                            sprintf(tmp, "%d", ele.status);
                            strcat(str, tmp);
                            append(str);

                            insertAtHead2(ele);
                            traversingList2(root2);
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "Register Successful\n");
                            send_message(buffer, cli->uid);
                        }
                    }
                    if (strstr(buffer, "LOGOUT"))
                    {
                        if (isLogin == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "Login First\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            p = strtok(buffer, "|");
                            char us[100];
                            strcpy(us, strtok(NULL, "|"));

                            isLogin = 0;
                            cli->elo = 0; /*cap nhat elo client*/
                            strcpy(cli->name, cli->username);
                            userNode *n;
                            for (n = root2; n != NULL; n = n->next)
                            {
                                if (strcmp(us, n->element.name) == 0)
                                {
                                    n->element.status = 0;
                                    //displayNode2(n);
                                    saveData1(n->element);
                                    // TODO : luu data vaofile khi logout
                                    // saveData("database.txt",n->element);
                                }
                            }
                            traversingList2(root2);
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "Logout Successful\n");
                            send_message(buffer, cli->uid);
                            // TODO:save data vao file database
                        }
                    }
                    if (strstr(buffer, "LOGIN"))
                    {
                        if (isLogin == 1)
                        {
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "Already login.Logged out first\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            int fl = 1;
                            userNode *n;
                            p = strtok(buffer, "|");
                            strcpy(user, strtok(NULL, "|"));
                            printf("userName received :%s\n", user);
                            strcpy(pass, strtok(NULL, "|"));
                            printf("password received :%s\n", pass);
                            for (n = root2; n != NULL; n = n->next)
                            {
                                if (strcmp(user, n->element.name) == 0 && strcmp(pass, n->element.pass) == 0)
                                {
                                    if (n->element.status == 0)
                                    {
                                        // TODO: them 1 ham isLogin = status (0 la chua log)
                                        isLogin = 1;
                                        n->element.status = 1;
                                        cli->elo = n->element.elo;
                                        strcpy(cli->username, cli->name);
                                        strcpy(cli->name, user);
                                        // cli->userInfo = (p->element);
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
                                traversingList2(root2);
                                bzero(buffer, BUFFER_SZ);
                                strcpy(buffer,"USER|");
                                strcat(buffer, user);
                                send_message(buffer, cli->uid); //login succes
                            }
                            else if (fl == -1)
                            {

                                bzero(buffer, BUFFER_SZ);
                                sprintf(buffer, "Already login\n");
                                send_message(buffer, cli->uid);
                            }
                            else
                            {
                                bzero(buffer, BUFFER_SZ);
                                sprintf(buffer, "Login failure\n");
                                send_message(buffer, cli->uid);
                            }
                        }
                    }

                    if (strcmp(buffer, "create") == 0 || strcmp(buffer, "create rank") == 0)
                    {
                        if (strcmp(buffer, "create rank") == 0 && isLogin == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "You need to login\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            flag = 0;

                            pthread_mutex_lock(&rooms_mutex);

                            for (int i = 0; i < MAX_ROOMS; i++)
                            {
                                if (rooms[i])
                                {
                                    if (rooms[i]->player1->uid == cli->uid)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "[SERVER] you are already in the room\n");
                                        send_message(buffer, cli->uid);
                                        flag = 1;
                                        break;
                                    }

                                    if (rooms[i]->player2 != 0)
                                    {
                                        if (rooms[i]->player2->uid == cli->uid)
                                        {
                                            bzero(buffer, BUFFER_SZ);
                                            sprintf(buffer, "[SERVER] you are already in the room\n");
                                            send_message(buffer, cli->uid);
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
                                printf("Roomid create:%d\n", room->uid);
                                if (strcmp(buffer, "create") == 0)
                                {
                                    strcpy(room->roomType, "[NORMAL] ");
                                }
                                else
                                    strcpy(room->roomType, "[RANK] ");
                                strcpy(room->state, room->roomType);
                                strcat(room->state, "waiting for secound player");

                                // add room to queue
                                queue_add_room(room);
                                bzero(buffer, BUFFER_SZ);
                                sprintf(buffer, "[SERVER] you created a new room number %i\n", roomUid);
                                send_message(buffer, cli->uid);
                                roomUid++;
                            }
                        }
                    }

                    // random
                    else if (strcmp(command, "join") == 0)
                    {
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

                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] you are already in the room number: %i\n", rooms[j]->uid);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);

                        if (already == 1)
                        {
                            continue;
                        }

                        pthread_mutex_lock(&rooms_mutex);

                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                if (rooms[i]->uid == number)
                                {
                                    researched = 1;

                                    if (rooms[i]->player2 != 0)
                                    {
                                        if (rooms[i]->player2->uid == cli->uid)
                                        {
                                            bzero(buffer, BUFFER_SZ);
                                            send_message("[SERVER] you are already in the room\n", cli->uid);
                                            break;
                                        }

                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "[SERVER] room number: %i, is already full\n", rooms[i]->uid);
                                        send_message(buffer, cli->uid);
                                        break;
                                    }

                                    rooms[i]->player2 = cli;

                                    strcpy(rooms[i]->state, rooms[i]->roomType);
                                    strcat(rooms[i]->state, "waiting start");
                                    bzero(buffer, BUFFER_SZ);
                                    printf("%s enter the room number: %i\n", cli->name, number);
                                    sprintf(buffer, "[SERVER] '%s' entered your room\n", cli->name);
                                    send_message(buffer, rooms[i]->player1->uid);

                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] you has entered the room number: %i\n", number);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);

                        if (researched == 0)
                        {
                            sprintf(buffer, "[SERVER] could not find the room number %i\n", number);
                            send_message(buffer, cli->uid);
                        }
                    }
                    else if (strcmp(command, "list") == 0)
                    {
                        pthread_mutex_lock(&rooms_mutex);

                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                char *list = (char *)malloc(BUFFER_SZ * sizeof(char));

                                if (rooms[i]->player2 != 0)
                                {
                                    sprintf(list, "%i)\n    room state: %s  \n    player1: %s - elo: %d\n    player2: %s - elo: %d\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->name, rooms[i]->player1->elo, rooms[i]->player2->name, rooms[i]->player2->elo);
                                }
                                else
                                {
                                    sprintf(list, "%i)\n    room state: %s \n    player1: %s - elo: %d\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->name, rooms[i]->player1->elo);
                                }

                                send_message(list, cli->uid);
                                free(list);
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);
                    }
                    else if (strcmp(command, "leave") == 0)
                    {
                        int remove_room = 0;
                        int room_number = 0;

                        pthread_mutex_lock(&rooms_mutex);
                        printf("HEllo\n");
                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                if (rooms[i]->player1->uid == cli->uid)
                                {
                                    if (rooms[i]->player2 != 0)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "[SERVER] %s left the room, now you are the owner\n", rooms[i]->player1->name);
                                        send_message(buffer, rooms[i]->player2->uid);

                                        rooms[i]->player1 = rooms[i]->player2;
                                        rooms[i]->player2 = 0;
                                        strcpy(rooms[i]->state, rooms[i]->roomType);
                                        strcat(rooms[i]->state, "waiting for secound player");
                                    }
                                    else
                                    { // ko co ai
                                    // TODO : xoa room 1 trc roi ms xoa dc room 2
                                        remove_room = 1;
                                        room_number = rooms[i]->uid;
                                    }

                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] you left the room %i\n", rooms[i]->uid);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                                else if (rooms[i]->player2->uid == cli->uid)
                                {
                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] %s left the room\n", rooms[i]->player1->name);
                                    send_message(buffer, rooms[i]->player1->uid);

                                    rooms[i]->player2 = 0;
                                    strcpy(rooms[i]->state, rooms[i]->roomType);
                                    strcat(rooms[i]->state, "waiting for secound player");

                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] you left the room %i\n", rooms[i]->uid);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);
                        printf("HEllo1235\n");
                        if (remove_room == 1)
                        {
                            printf("Room delete:%d\n", room_number);
                            queue_remove_room(room_number);
                            roomUid++;
                        }
                    }
                    else if (strcmp(command, "start") == 0)
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

                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] 2 players are required to start the game\n");
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                                else if (rooms[j]->player2->uid == cli->uid)
                                {
                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "[SERVER] only the owner of the room can start\n");
                                    send_message(buffer, cli->uid);
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
                            strcpy(room_game->state, room_game->roomType);

                            strcat(room_game->state, "playing now");

                            for (int linha = 0; linha < 3; linha++)
                            {
                                for (int coluna = 0; coluna < 3; coluna++)
                                {
                                    room_game->game->board[linha][coluna] = '-';
                                }
                            }

                            sleep(1);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "start game\n");
                            send_message(buffer, room_game->player1->uid);

                            sleep(0.1);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "start game2\n");
                            send_message(buffer, room_game->player2->uid);

                            sleep(1);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "%s\n", room_game->player2->name);
                            send_message(buffer, room_game->player1->uid);

                            sleep(0.1);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "%s\n", room_game->player1->name);
                            send_message(buffer, room_game->player2->uid);

                            sleep(1);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "vez1\n");
                            send_message(buffer, room_game->player1->uid);

                            sleep(0.2);

                            bzero(buffer, BUFFER_SZ);
                            sprintf(buffer, "vez2\n");
                            send_message(buffer, room_game->player2->uid);
                        }
                    }
                    else if (strcmp(command, "play") == 0)
                    {
                        pthread_mutex_lock(&rooms_mutex);

                        for (int j = 0; j < MAX_ROOMS; j++)
                        {
                            if (rooms[j])
                            {
                                if (rooms[j]->player1->uid == cli->uid || rooms[j]->player2->uid == cli->uid)
                                {
                                    if (rooms[j]->game->gameStatus == 0)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "[SERVER] game over\n");
                                        send_message(buffer, cli->uid);
                                        break;
                                    }

                                    setbuf(stdin, 0);
                                    bzero(buffer, BUFFER_SZ);
                                    sprintf(buffer, "%i", number);

                                    int linhaJogada = posicoes[number - 1][0];
                                    int colunaJogada = posicoes[number - 1][1];
                                    if (rooms[j]->game->playerTurn == rooms[j]->player1->uid)
                                    {
                                        send_message(buffer, rooms[j]->player2->uid);
                                        rooms[j]->game->board[linhaJogada][colunaJogada] = 'X';
                                        rooms[j]->game->playerTurn = rooms[j]->player2->uid;
                                    }
                                    else if (rooms[j]->game->playerTurn == rooms[j]->player2->uid)
                                    {
                                        send_message(buffer, rooms[j]->player1->uid);
                                        rooms[j]->game->board[linhaJogada][colunaJogada] = 'O';
                                        rooms[j]->game->playerTurn = rooms[j]->player1->uid;
                                    }

                                    for (int iterator = 0; iterator < 3; iterator++)
                                    {
                                        if (
                                            (
                                                (rooms[j]->game->board[iterator][0] == rooms[j]->game->board[iterator][1]) && (rooms[j]->game->board[iterator][1] == rooms[j]->game->board[iterator][2]) && rooms[j]->game->board[iterator][0] != '-') ||
                                            ((rooms[j]->game->board[0][iterator] == rooms[j]->game->board[1][iterator]) && (rooms[j]->game->board[1][iterator] == rooms[j]->game->board[2][iterator]) && rooms[j]->game->board[0][iterator] != '-'))
                                        {
                                            rooms[j]->game->gameStatus = 0;
                                        }
                                    }

                                    if (
                                        (
                                            (rooms[j]->game->board[0][0] == rooms[j]->game->board[1][1]) && (rooms[j]->game->board[1][1] == rooms[j]->game->board[2][2]) && rooms[j]->game->board[0][0] != '-') ||
                                        ((rooms[j]->game->board[0][2] == rooms[j]->game->board[1][1]) && (rooms[j]->game->board[1][1] == rooms[j]->game->board[2][0]) && rooms[j]->game->board[0][2] != '-'))
                                    {
                                        rooms[j]->game->gameStatus = 0;
                                    }

                                    sleep(1);

                                    if (rooms[j]->game->gameStatus == 0)
                                    {
                                        strcpy(rooms[j]->state, rooms[j]->roomType);
                                        strcat(rooms[j]->state, "waiting start");

                                        if (rooms[j]->game->playerTurn == rooms[j]->player1->uid)
                                        {
                                            bzero(buffer, BUFFER_SZ);
                                            sprintf(buffer, "win1\n");
                                            send_message(buffer, rooms[j]->player1->uid);

                                            sleep(0.5);

                                            send_message(buffer, rooms[j]->player2->uid);
                                        }
                                        else if (rooms[j]->game->playerTurn == rooms[j]->player2->uid)
                                        {
                                            bzero(buffer, BUFFER_SZ);
                                            sprintf(buffer, "win2\n");
                                            send_message(buffer, rooms[j]->player1->uid);

                                            sleep(0.5);

                                            send_message(buffer, rooms[j]->player2->uid);
                                        }

                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "ok");
                                        send_message(buffer, rooms[j]->player1->uid);

                                        sleep(0.5);

                                        send_message(buffer, rooms[j]->player2->uid);
                                        break;
                                    }

                                    if (rooms[j]->game->playerTurn == rooms[j]->player1->uid)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "vez1\n");
                                        send_message(buffer, rooms[j]->player1->uid);

                                        sleep(0.5);

                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "vez2\n");
                                        send_message(buffer, rooms[j]->player2->uid);
                                    }
                                    else if (rooms[j]->game->playerTurn == rooms[j]->player2->uid)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "vez2\n");
                                        send_message(buffer, rooms[j]->player1->uid);

                                        sleep(0.5);

                                        bzero(buffer, BUFFER_SZ);
                                        sprintf(buffer, "vez1\n");
                                        send_message(buffer, rooms[j]->player2->uid);
                                    }

                                    rooms[j]->game->round++;
                                }
                                break;
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);
                    }
                }
        }
        else if (receive == 0 || strcmp(buffer, "exit") == 0)
        {
            sprintf(buffer, "%s has left\n", cli->name);
            printf("%s", buffer);
            // send_message(buffer, cli->uid);
            leave_flag = 1;
        }
        else
        {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buffer, BUFFER_SZ);
    }

    bzero(buffer, BUFFER_SZ);
    sprintf(buffer, "bye");
    send_message(buffer, cli->uid);

    close(cli->sockfd);
    queue_remove_client(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // Socket settings
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    // Signals
    signal(SIGPIPE, SIG_IGN);

    if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
    {
        printf("ERROR: setsockopt\n");
        return EXIT_FAILURE;
    }

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error bind\n");
        return EXIT_FAILURE;
    }

    // listen
    if (listen(listenfd, 10) < 0)
    {
        printf("ERROR: listen\n");
        return EXIT_FAILURE;
    }

    flashScreen();

    printf("############################################");
    printf("\n# Tic-Tac-Toe Server running on port: %i #", port);
    printf("\n############################################\n\n");
    importTextFile("database.txt");
    traversingList2(root2);
    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

        // check dor max clients
        if ((cli_count + 1) == MAX_CLIENTS)
        {
            printf("Maximun of clients are connected, Connection rejected");
            close(connfd);
            continue;
        }
        // printf("New connection:%d\n", connfd);
        //  clients settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;
        printf("Uid:%d\n", cli->uid);
        // add client to queue
        queue_add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        // reduce CPU usage
        sleep(1);
    }

    return EXIT_SUCCESS;
}
