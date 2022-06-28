#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/types.h>
#include "utils/customSTD.h"
#include <math.h>
#include "utils/user.h"
#include "authenticate.h"

#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define BUFFER_SZ 2048
#define NAME_LEN 100
#define KEY 0XAED
// Symmetric 1 key for encrypt and decrypt
static _Atomic unsigned int cli_count = 0;
static int uid = 10;
static int roomUid = 1;
int firstElo, secondElo;

int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

// client structure
#include "utils/room.h"

client_t *clients[MAX_CLIENTS];
room_t *rooms[MAX_ROOMS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t files_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "utils/queueManager.h"

int i = 0;

float Probability(int rating1, int rating2)
{
    return 1.0 * 1.0 / (1 + 1.0 * pow(10, 1.0 * (rating1 - rating2) / 400));
}

void EloRating(int Ra, int Rb, int K, int d)
{
    float Pb = Probability(Ra, Rb);

    float Pa = Probability(Rb, Ra);

    int a, b;
    if (d == 1)
    {
        firstElo = Ra + K * (1 - Pa);
        secondElo = Rb + K * (0 - Pb);
    }

    else
    {
        firstElo = Ra + K * (0 - Pa);
        secondElo = Rb + K * (1 - Pb);
    }
    /*fflush(stdout);
    printf( "Ra = %d Rb = %d", Ra,Rb );*/
}

void send_message(char *message, int uid)
{

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                // ass res.mess res.status
                // res.status = status;
                // strcpy(res.message, message);
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

#include "server_auth/server_auth.h"
#include "server_game/server_game.h"
void *handle_client(void *arg)
{
    char buffer[BUFFER_SZ];
    char command[BUFFER_SZ];
    char tmp[BUFFER_SZ];
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

    cli->userInfo.elo = 1200;
    cli->userInfo.status = 0;
    strcpy(cli->userInfo.name, "unknown");
    sprintf(buffer, "> %s has joined\n", cli->userInfo.name);
    printf("%s", buffer);

    bzero(buffer, BUFFER_SZ);
    strcpy(buffer, "SELECT_MODE|");
    strcat(buffer, "ok1");
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
                printf("> client: '%s' has been send '%s' command\n", cli->userInfo.name, buffer);
                sscanf(buffer, "%[^|]|%i", &command[0], &number);
                
                if (strstr(buffer, "GUEST"))
                {
                    handleGuest(name, cli, buffer);
                   
                }
                else if (strstr(buffer, "SIGNUP"))
                { // TODO:luu vao file
                    handleReg(cli, buffer);
                }
                else if (strstr(buffer, "LOGOUT"))
                {
                    handleLogOut(&isLogin, cli, buffer);
                }
                else if (strstr(buffer, "LOGIN"))
                {
                    handleLogin(&isLogin, cli, buffer);
                }

                else if (strcmp(buffer, "CREATE") == 0 || strcmp(buffer, "CREATE RANK") == 0)
                {
                    handleCreateRoom(&isLogin, &flag, cli, buffer);
                }

                // random
                else if (strcmp(command, "JOIN") == 0)
                {
                    handleJoin(&isLogin, &number, cli);
                }
                else if (strcmp(command, "LIST") == 0)
                {
                    handleListRooms(cli);
                }
                else if (strcmp(command, "LEAVE") == 0)
                {
                    handleLeave(cli);
                }
                else if (strcmp(command, "START") == 0)
                {
                    handleStart(cli);
                }
                else if (strcmp(command, "PLAY") == 0)
                {
                    handlePlay(&number, cli);
                }
                else
                {
                    bzero(buffer, BUFFER_SZ);
                    strcpy(buffer, "INVALID_CMD|");
                    strcat(buffer, "Invalid command\n");
                    send_message(buffer, cli->uid);
                }
            }
        }
        else if (receive == 0 || strcmp(buffer, "exit") == 0)
        {
            // TODO: THEM PHAN LOGOUT VAO DAY
            sprintf(buffer, "%s has left\n", cli->userInfo.name);
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
    strcpy(buffer, "EXIT|");
    strcat(buffer, "bye");
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

    // char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // Socket settings
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Signals
    signal(SIGPIPE, SIG_IGN);
    /*
        if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
        {
            printf("ERROR: setsockopt\n");
            return EXIT_FAILURE;
        }*/

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
