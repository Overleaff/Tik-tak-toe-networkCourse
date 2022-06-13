#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "protocol.h"
#include <pthread.h>

#define HOSTPORTNUM 27429 /* Local port number of the client machine that peers will use to connect  */
#define PEERPORTNUM 27429 /* Peer(Opponent) port number */
#define STRLEN 1024
char name[STRLEN],    /* name used to join the server  */
    opponent[STRLEN]; /* name of the peer(Opponent)  */
pthread_t tid;        /*Thread ID*/
int yours = 0;        /* User's score*/
int oppos = 0;        /* Peer's(Opponent) Score*/
int ingame, turn;     /* Some variables to handle game logic. */

void *serverthread(void *parm); /* Thread function to handle communication with the server */
                                /* Thread function used when we want to open a connection with a peer */
                                /* All socket descriptors are stored as global variables */

/* Some variables to handle game logic. */
char message[1000];
char user[100] = "";
response res;
//////////////////////////
// bodrd
#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NAME_LEN 32

pthread_t multiplayer_game;
int serversocket = 0;

int player = 1;
void showPositions()
{
    printf("\nPosition map:");
    printf("\n 7 | 8 | 9");
    printf("\n 4 | 5 | 6");
    printf("\n 1 | 2 | 3\n");
}

void showBoard(char tabuleiro[3][3], char *errorMessage)
{
    int linha;

    if (*errorMessage != '\x00')
    {
        printf("Warning: %s!\n\n", errorMessage);
        *errorMessage = '\x00';
    }

    printf("#############\n");

    for (linha = 0; linha < 3; linha++)
    {
        printf("# %c | %c | %c #", tabuleiro[linha][0], tabuleiro[linha][1], tabuleiro[linha][2]);

        printf("\n");
    }

    printf("#############\n");

    showPositions();
}

void createBoard(char tabuleiro[3][3])
{
    int linha, coluna;

    for (linha = 0; linha < 3; linha++)
    {
        for (coluna = 0; coluna < 3; coluna++)
        {
            tabuleiro[linha][coluna] = '-';
        }
    }
}

void *multiplayerGame(void *arg)
{
    int turn;
    turn = player;
    char namePlayer1[32];
    char namePlayer2[32];
     
    strcpy(namePlayer1, name);

    char tabuleiro[3][3];
    int iterator;
    int linhaJogada, colunaJogada;
    int positionPlay;
    int rodada = 0;
    int gameStatus = 1;
    int valid_play = 0;
    int played;
    int numberPlayed;
    int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

    char errorMessage[255] = {'\x00'};
    char *nameCurrentPlayer;
    char message[BUFFER_SZ] = {};

    response res;
    int receive = recv(serversocket, &res, sizeof(res), 0);

    if (receive > 0)
    {
        setbuf(stdin, 0);

        sscanf(res.message, "%s", &namePlayer2[0]);

        setbuf(stdout, 0);
        setbuf(stdin, 0);

        // bzero(res.message, BUFFER_SZ);

        createBoard(tabuleiro);
        rodada = 0;

        while (rodada < 9 && gameStatus == 1)
        {
            if (turn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
            }
            else
            {
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            showBoard(tabuleiro, (char *)&errorMessage);

            printf("\nRound: %d", rodada);
            printf("\nPlayer: %s\n", nameCurrentPlayer);

            while (valid_play == 0)
            {
                // bzero(message, BUFFER_SZ);

                int receive = recv(serversocket, &res, sizeof(res), 0);

                if (receive > 0)
                {
                    valid_play = 1;

                    setbuf(stdin, 0);
                    setbuf(stdout, 0);

                    if (strcmp(res.message, "vez1\n") == 0)
                    {
                        printf("Enter a position: ");
                        scanf("%d", &positionPlay);

                        linhaJogada = posicoes[positionPlay - 1][0];
                        colunaJogada = posicoes[positionPlay - 1][1];

                        if (valid_play == 1)
                        {   
                            res.status=200;
                            sprintf(res.message, "play %i\n", positionPlay);
                            send(serversocket, &res, sizeof(res), 0);
                            // bzero(message, BUFFER_SZ);
                        }
                    }
                    else if (strcmp(res.message, "vez2\n") == 0)
                    {
                        printf("The other player is playing...\n");

                        played = 0;

                        while (played == 0)
                        {
                            int receive = recv(serversocket, &res, sizeof(res), 0);

                            if (receive > 0)
                            {
                                sscanf(res.message, "%i", &numberPlayed);

                                linhaJogada = posicoes[numberPlayed - 1][0];
                                colunaJogada = posicoes[numberPlayed - 1][1];

                                played = 1;
                            }
                        }

                        valid_play = 1;
                    }
                }
                else
                {
                    valid_play = 0;
                }
            }

            if (turn == 1)
            {
                tabuleiro[linhaJogada][colunaJogada] = 'X';
                turn = 2;
            }
            else
            {
                tabuleiro[linhaJogada][colunaJogada] = 'O';
                turn = 1;
            }

            for (iterator = 0; iterator < 3; iterator++)
            {
                if (
                    (
                        (tabuleiro[iterator][0] == tabuleiro[iterator][1]) && (tabuleiro[iterator][1] == tabuleiro[iterator][2]) && tabuleiro[iterator][0] != '-') ||
                    ((tabuleiro[0][iterator] == tabuleiro[1][iterator]) && (tabuleiro[1][iterator] == tabuleiro[2][iterator]) && tabuleiro[0][iterator] != '-'))
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (tabuleiro[0][0] == tabuleiro[1][1]) && (tabuleiro[1][1] == tabuleiro[2][2]) && tabuleiro[0][0] != '-') ||
                ((tabuleiro[0][2] == tabuleiro[1][1]) && (tabuleiro[1][1] == tabuleiro[2][0]) && tabuleiro[0][2] != '-'))
            {
                gameStatus = 0;
            }

            rodada++;
            valid_play = 0;
           // bzero(message, BUFFER_SZ);
        }

       // bzero(message, BUFFER_SZ);

        int receive = recv(serversocket, &res, sizeof(res), 0);

        if (receive > 0)
        {
            setbuf(stdin, 0);
            setbuf(stdout, 0);

            showBoard(tabuleiro, (char *)&errorMessage);

            if (strcmp(res.message, "win1\n") == 0)
            {
                printf("\nPlayer '%s' win!", nameCurrentPlayer);
            }
            else if (strcmp(res.message, "win2\n") == 0)
            {
                printf("\nPlayer '%s' win!", nameCurrentPlayer);
            }

            printf("\nEnd of the game!\n");

            sleep(6);

            pthread_detach(pthread_self());
            pthread_cancel(multiplayer_game);
        }
    }

    return NULL;
}
//////////////////////////
void menu()
{
    char choice[10];
    memset(choice, '\0', (strlen(choice) + 1));
    printf("=====================================\n");
    printf("1. Login.\n");
    printf("2. Create new account.\n");
    printf("3. Exit.\n");
    printf("=====================================\n");

    printf("=>>You enter into the selection:");
    scanf("%s", choice);
    printf("-------------------------------------\n");
    int c = atoi(choice);
    switch (c)
    {
    case 1:

    case 2:
    case 3:
    {

        memset(message, '\0', (strlen(message) + 1));
        strcat(message, "SELECT_WORK|");
        strcat(message, choice);
        strcat(message, "|");

        break;
    }
    default:
    {
        printf("**ERORR** Please re-enter your selection.\n");
        menu();
    }
    }
}
void lg_user()
{
    memset(user, '\0', (strlen(user) + 1));
    userLogin userInfo;
    printf("Enter userId : ");
    scanf("%s", userInfo.user);
    printf("Enter password : ");
    scanf("%s", userInfo.pass);
    memset(message, '\0', (strlen(message) + 1));
    strcat(message, "LOGIN|");
    strcat(message, userInfo.user);
    strcpy(user, userInfo.user);
    strcat(message, "|");
    strcat(message, userInfo.pass);
}
void sgnUp_menu()
{
    userLogin userInfo;
    printf("Enter userId : ");
    scanf("%s", userInfo.user);
    printf("Enter password : ");
    scanf("%s", userInfo.pass);
    memset(message, '\0', (strlen(message) + 1));
    strcat(message, "SIGNUP|");
    strcat(message, userInfo.user);
    strcat(message, "|");
    strcat(message, userInfo.pass);
}
void authenticated_menu()
{
    int choice;
    int dd = 0;
    printf("=====================================\n");

    printf("1|- Start game\n");
    printf("2|- Logout.\n");
    printf("3|- Exit.\n");
    printf("4|list\t\t   List all tic-tac-toe rooms\n");
    printf("5|create  Create one tic-tac-toe room\n");
    printf("6|join {room number}\t  Join in one tic-tac-toe room\n");
    printf("7|leave\t\t  Back of the one tic-tac-toe room\n");
    printf("8|start\t\t  Starts one tic-tac-toe game\n\n");
    printf("=====================================\n");

    printf("=>>You enter into the selection:");
    scanf("%d", &choice);
    while (getchar() != '\n')
        ;
    memset(message, '\0', (strlen(message) + 1));
    switch (choice)
    {
    case 1:
    {
        strcat(message, "START_GAME|");
        strcat(message, user);
        strcat(message, "|");

        break;
    }
    case 2:
    {
        strcat(message, "LOGOUT|");
        strcat(message, user);
        strcat(message, "|");
        printf("You are logged out\n");
        break;
    }
    case 3:
    {

        strcat(message, "SELECT_WORK|");
        strcat(message, "4");
        strcat(message, "|");
        strcat(message, user);
        strcat(message, "|");

        break;
    }
    case 4:
    {

        strcat(message, "list|");

        break;
    }
    case 5:
    {

        strcat(message, "create|");

        break;
    }
    case 6:
    {

        strcat(message, "join|");
        char str[2];
        printf("Enter room number to join:");
        scanf("%s", str);
        strcat(message, str);
        strcat(message, "|");

        break;
    }
    case 7:
    {

        strcat(message, "leave|");

        break;
    }
    case 8:
    {

        strcat(message, "start|");

        break;
    }
    default:
    {
        printf("**ERORR** Please re-enter your selection.\n");
        authenticated_menu();
    }
    }
}
int check_message(response res)
{
    if (strcmp(res.message, "Start") == 0)
    {
        menu();
        return 1;
    }
    if (strcmp(res.message, "Login") == 0 || strcmp(res.message, "Login Failure") == 0 || strcmp(res.message, "Already Login") == 0)
    {
        lg_user();
        return 1;
    }
    if (strcmp(res.message, "Login Successful") == 0)
    {
        printf("You are logged in\n");
        authenticated_menu();
        return 1;
    }
    if (strcmp(res.message, "SignUp") == 0 || strcmp(res.message, "Register Fail") == 0)
    {

        sgnUp_menu();
        return 1;
    }
    if (strcmp(res.message, "Register Successful") == 0)
    {

        printf("You are register in\n");
        menu();
        return 1;
    }
    if (strcmp(res.message, "Exit") == 0)
    {

        printf("Good bye\n");

        return 0;
    }

    if (strstr(res.message, "[SERVER] you created a new room") || strstr(res.message, "is already full") || strstr(res.message, "entered your room") || strstr(res.message, "you has entered the room") || strstr(res.message, "could not find the room") || strstr(res.message, "now you are the owner"))
    {
       
        authenticated_menu();
        return 1;
    }
    if (strstr(res.message, "[SERVER] you left the room") || strstr(res.message, "already in the room") || strstr(res.message, "room state") ||
        strstr(res.message, "2 players are required") || strstr(res.message, "only the owner of"))
    {

        authenticated_menu();
        return 1;
    }
    if (strcmp(res.message, "start game\n") == 0)
    {

        player = 1;
        if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
        {
            printf("ERROR: pthread\n");
            exit(EXIT_FAILURE);
        }
        pthread_detach(pthread_self());
    }
    if (strcmp(res.message, "start game2\n") == 0)
    {

        player = 2;
        if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
        {
            printf("ERROR: pthread\n");
            exit(EXIT_FAILURE);
        }
        pthread_detach(pthread_self());
    }
    return 0;
}
int main(int argc, char *argv[])
{

    struct sockaddr_in server;

    int byte_sent, byte_received;
    char *SERV_IP;
    short SERV_PORT;

    response res;
    if (argc != 3)
    {
        printf("Usage: %s <IPAddress> <PortNumber> ", argv[0]);
        exit(1);
    }
    SERV_IP = argv[1];
    SERV_PORT = atoi(argv[2]);
    // printf("%s",SERV_IP);
    /* Establish connection with the server */
    // Step 1: Construct socket
    if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: ");
        return 0;
    }
    // Step 2: Define the address of the server
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERV_PORT);
    server.sin_addr.s_addr = htonl(inet_network(SERV_IP));

    printf("CLIENT START\n");

    int ret = connect(serversocket, (struct sockaddr *)&server, sizeof(server));
    if (ret < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");
    while (1)
    {
        int byte_received = recv(serversocket, &res, sizeof(res), 0);
        if (byte_received == -1)
        {
            printf("\nError!Cannot receive data from sever!\n");
            close(serversocket);
            exit(-1);
        }
        else
            printf("[%d]-%s\n", res.status, res.message);

        if (check_message(res) == 1)
        {
            int byte_sent = send(serversocket, message, strlen(message), 0);
            if (byte_sent <= 0)
            {
                printf("\nError!Cannot send data to sever!\n");
                close(serversocket);
                exit(-1);
            }
        }
        else
        {

            close(serversocket);
            exit(1);
            break;
        }
    }

    close(serversocket);
    return 0;
}
