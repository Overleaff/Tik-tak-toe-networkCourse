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

#include "customSTD.h"

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NAME_LEN 32
typedef struct protocol2
{
    int status;
    char message[1000];
} response;
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[NAME_LEN];
int player = 1;

pthread_t lobby_thread;
pthread_t recv_msg_thread;
pthread_t multiplayer_game;

char *ip = "127.0.0.1";
int port = 2050;
void startScreen();
void catch_ctrl_c_and_exit()
{
    flag = 1;
}

void showPositions()
{
    printf("\nPosition map:");
    printf("\n 7 | 8 | 9");
    printf("\n 4 | 5 | 6");
    printf("\n 1 | 2 | 3\n");
}

void showBoard(char board[3][3], char *errorMessage)
{
    int linha;

    flashScreen();

    if (*errorMessage != '\x00')
    {
        printf("Warning: %s!\n\n", errorMessage);
        *errorMessage = '\x00';
    }

    printf("#############\n");

    for (linha = 0; linha < 3; linha++)
    {
        printf("# %c | %c | %c #", board[linha][0], board[linha][1], board[linha][2]);

        printf("\n");
    }

    printf("#############\n");

    showPositions();
}

void createBoard(char board[3][3])
{
    int linha, coluna;

    for (linha = 0; linha < 3; linha++)
    {
        for (coluna = 0; coluna < 3; coluna++)
        {
            board[linha][coluna] = '-';
        }
    }
}

void menu();

void *lobby(void *arg);

void recv_msg_handler();

void *multiplayerGame(void *arg)
{
    int playerTurn;
    playerTurn = player; // ngchoi 1 hay 2
    char namePlayer1[32];
    char namePlayer2[32];

    strcpy(namePlayer1, name);

    char board[3][3];
    int iterator;
    int linePlay, colPlay;
    int positionPlay;
    int round = 0;
    int gameStatus = 1;
    int valid_play = 0;
    int played;
    int numberPlayed;
    int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

    char errorMessage[255] = {'\x00'};
    char *nameCurrentPlayer;
    char message[BUFFER_SZ] = {};
    response res;

    int receive = recv(sockfd, &res, sizeof(res), 0);
    strcpy(message, res.message);
    if (receive > 0)
    {
        setbuf(stdin, 0);
        trim_lf(message, strlen(message));
        sscanf(message, "%s", &namePlayer2[0]);

        setbuf(stdout, 0);
        setbuf(stdin, 0);

        bzero(message, BUFFER_SZ);

        createBoard(board);
        round = 0;

        while (round < 9 && gameStatus == 1)
        {
            if (playerTurn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
            }
            else
            {
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            showBoard(board, (char *)&errorMessage);

            printf("\nRound: %d", round);
            printf("\nPlayer: %s\n", nameCurrentPlayer);

            while (valid_play == 0)
            {
                bzero(message, BUFFER_SZ);

                int receive = recv(sockfd, &res, sizeof(res), 0);
                strcpy(message, res.message);
                if (receive > 0)
                {
                    valid_play = 1;

                    setbuf(stdin, 0);
                    setbuf(stdout, 0);
                    // your turn
                    if (strcmp(message, "vez1\n") == 0)
                    {
                        do
                        {
                            printf("Enter a position: ");
                            scanf("%d", &positionPlay);

                            linePlay = posicoes[positionPlay - 1][0];
                            colPlay = posicoes[positionPlay - 1][1];

                            if (board[linePlay][colPlay] == 'X' || board[linePlay][colPlay] == 'O')
                            {
                                printf("Invalid move\n");
                            }

                        } while (board[linePlay][colPlay] == 'X' || board[linePlay][colPlay] == 'O');
                        if (valid_play == 1)
                        {
                            sprintf(message, "play %i\n", positionPlay);
                            send(sockfd, message, strlen(message), 0);
                            bzero(message, BUFFER_SZ);
                        }
                    }
                    else if (strcmp(message, "vez2\n") == 0)
                    {
                        printf("The other player is playing...\n");

                        played = 0;

                        while (played == 0)
                        {
                            int receive = recv(sockfd, &res, sizeof(res), 0);
                            strcpy(message, res.message);

                            if (receive > 0)
                            {
                                sscanf(message, "%i", &numberPlayed);

                                linePlay = posicoes[numberPlayed - 1][0];
                                colPlay = posicoes[numberPlayed - 1][1];

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

            if (playerTurn == 1)
            {
                board[linePlay][colPlay] = 'X';
                playerTurn = 2;
            }
            else
            {
                board[linePlay][colPlay] = 'O';
                playerTurn = 1;
            }

            for (iterator = 0; iterator < 3; iterator++)
            {
                if (
                    (
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-') ||
                    ((board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'))
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-') ||
                ((board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'))
            {
                gameStatus = 0;
            }

            round++;
            valid_play = 0;
            bzero(message, BUFFER_SZ);
        }

        bzero(message, BUFFER_SZ);

        int receive = recv(sockfd, &res, sizeof(res), 0);
        strcpy(message, res.message);
        if (receive > 0)
        {
            setbuf(stdin, 0);
            setbuf(stdout, 0);

            showBoard(board, (char *)&errorMessage);

            if (strcmp(message, "win1\n") == 0)
            {
                printf("\nPlayer '%s' win!", nameCurrentPlayer);
            }
            else if (strcmp(message, "win2\n") == 0)
            {
                printf("\nPlayer '%s' win!", nameCurrentPlayer);
            }

            printf("\nEnd of the game!\n");

            sleep(6);

            if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            pthread_detach(pthread_self());
            pthread_cancel(multiplayer_game);
        }
    }

    return NULL;
}
char username[100];  // hold name temp when not login
void *lobby(void *arg)
{
    char buffer[BUFFER_SZ] = {};
    char uname[100], pass[100];
    while (1)
    {
        str_overwrite_stdout();
        fgets(buffer, BUFFER_SZ, stdin);

        trim_lf(buffer, BUFFER_SZ);
        if (strcmp(buffer, "guest") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            printf("Enter name : ");
            scanf("%s", uname);
            memset(buffer, '\0', (strlen(buffer) + 1));
            strcpy(buffer, "GUEST|");
            strcat(buffer, uname);
           
            strcpy(name,uname);
        }
        if (strcmp(buffer, "signup") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            memset(pass, '\0', (strlen(pass) + 1));
            printf("Enter userId : ");
            scanf("%s", uname);
            printf("Enter password : ");
            scanf("%s", pass);
            memset(buffer, '\0', (strlen(buffer) + 1));
            strcpy(buffer, "SIGNUP|");
            strcat(buffer, uname);
            strcat(buffer, "|");
            strcat(buffer, pass);
        }
        if (strcmp(buffer, "login") == 0)
        {

            printf("Enter userId : ");
            scanf("%s", uname);
            printf("Enter password : ");
            scanf("%s", pass);
            memset(buffer, '\0', (strlen(buffer) + 1));
            strcpy(buffer, "LOGIN|");
            strcat(buffer, uname);
            strcat(buffer, "|");
            strcat(buffer, pass);

            strcpy(name, uname);
        }
        if (strcmp(buffer, "logout") == 0)
        {
            printf("> Good bye %s", username);

            memset(buffer, '\0', (strlen(buffer) + 1));
            strcpy(buffer, "LOGOUT|");
            strcat(buffer, name);
            strcat(buffer, "|");
            strcpy(username, "");
            str_overwrite_stdout();
        }
        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }
        else
        {
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(buffer, BUFFER_SZ);
    }

    catch_ctrl_c_and_exit(2);

    return NULL;
}

void menu(){
    printf("Comandos:\n");
    printf("\t -list\t\t\t  List all tic-tac-toe rooms\n");
    printf("\t -create\t\t   Normal Room\n");
    printf("\t -create rank\t\t  Ranked room\n");
    printf("\t -join {room number}\t  Join in one tic-tac-toe room\n");
    printf("\t -leave\t\t\t  Back of the one tic-tac-toe room\n");
    printf("\t -start\t\t\t  Starts one tic-tac-toe game\n");
    printf("\t -login\t\t\t  Logged in to save your account\n");
    printf("\t -signup\t\t  Dont' have an account? Register\n");
    printf("\t -exit\t\t\t  Close terminal\n\n");
}
void logged_menu(){
    printf("Comandos:\n");
    printf("\t -list\t\t\t  List all tic-tac-toe rooms\n");
    printf("\t -create\t\t   Normal Room\n");
    printf("\t -create rank\t\t  Ranked room\n");
    printf("\t -join {room number}\t  Join in one tic-tac-toe room\n");
    printf("\t -leave\t\t\t  Back of the one tic-tac-toe room\n");
    printf("\t -start\t\t\t  Starts one tic-tac-toe game\n");
    printf("\t -logout\t\t  Starts one tic-tac-toe game\n\n");
}
void selectMode(){
    
    printf("guest  -login as guest\n");
    printf("login  -signin\n");
    printf("signup -register new account\n\n");
}
void recv_msg_handler()
{
    char message[BUFFER_SZ] = {};
    response res;
    flashScreen();

    while (1)
    {
        int receive = recv(sockfd, &res, sizeof(res), 0);
        strcpy(message, res.message);
        if (receive > 0)
        {
            if (strcmp(message, "ok1") == 0)
            { // TODO: THEM BIEN IS LOGIN DE THAY DOI TERMINAL
                selectMode();
                str_overwrite_stdout();
                
                
            }
            else if (strcmp(message, "ok") == 0)
            { //TODO: THEM BIEN IS LOGIN DE THAY DOI TERMINAL
                
                menu();
                str_overwrite_stdout();
            }
            else if (strcmp(message, "start game\n") == 0)
            {
                pthread_cancel(lobby_thread);
                // pthread_kill(recv_msg_thread, SIGUSR1);

                player = 1;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    printf("ERROR: pthread\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);

                // pthread_kill(lobby_thread, SIGUSR1);
            }
            else if (strcmp(message, "start game2\n") == 0)
            {
                pthread_cancel(lobby_thread);
                // pthread_kill(recv_msg_thread, SIGUSR1);

                player = 2;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    printf("ERROR: pthread\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);

                // pthread_kill(lobby_thread, SIGUSR1);
            }
            else if (res.status == 204)
            {
                printf("Login Successful\n");
                strcpy(username, name);
                strcpy(name, res.message);
                flashScreen();
                printf("Hello:%s\n", name);
                logged_menu();
                str_overwrite_stdout();
            }
            else if (strcmp("Logout Successful\n",res.message)==0){
                strcpy(name,username);
                flashScreen();
                menu();
                str_overwrite_stdout();
            }
                else
                {
                    printf("[%d] - %s", res.status, message);
                    str_overwrite_stdout();
                }
        }
        else if (receive == 0)
        {
            break;
        }

        bzero(message, BUFFER_SZ);
    }
}
/*
void send_msg_handler()
{
    char buffer[BUFFER_SZ] = {};

    while(1)
    {
        str_overwrite_stdout();
        fgets(buffer, BUFFER_SZ, stdin);
        trim_lf(buffer, BUFFER_SZ);

        if (strcmp(buffer, "exit") == 0) {
            break;
        } else {
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(buffer, BUFFER_SZ);
    }

    catch_ctrl_c_and_exit(2);
}
*/
int conectGame()
{
    setbuf(stdin, 0);
    //printf("1.Play as guest");
    //printf("2.Login");
    /*
    do
    {
        printf("Enter your name(guest): ");
        fgets(name, BUFFER_SZ, stdin);
        trim_lf(name, BUFFER_SZ);

        // strcpy(name, "murilo");

        if (strlen(name) > NAME_LEN - 1 || strlen(name) < 2)
        {
            printf("Enter name corretly (length>1)\n");
        }
    } while (strlen(name) > NAME_LEN - 1 || strlen(name) < 2);*/
    strcpy(name,"GO");
    struct sockaddr_in server_addr;

    // socket settings
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    // connect to the server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1)
    {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // send the name
    send(sockfd, name, NAME_LEN, 0);

    if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        if (flag)
        {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}

void game()
{
    char board[3][3];
    int iterator;
    int linePlay, colPlay;
    int positionPlay;
    int playerTurn = 1;
    int round = 0;
    int gameStatus = 1;
    int optionRestart;
    int gaming = 1;

    char errorMessage[255] = {'\x00'};

    char namePlayer1[32];
    char namePlayer2[32];
    char *nameCurrentPlayer;

    setbuf(stdin, 0);

    printf("1st player's name: ");
    fgets(namePlayer1, 32, stdin);

    namePlayer1[strlen(namePlayer1) - 1] = '\x00';

    printf("2nd player's name: ");
    fgets(namePlayer2, 32, stdin);

    namePlayer2[strlen(namePlayer2) - 1] = '\x00';

    while (gaming == 1)
    {
        createBoard(board);
        round = 0;
        playerTurn = 1;

        while (round < 9 && gameStatus == 1)
        {
            showBoard(board, (char *)&errorMessage);

            if (playerTurn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
            }
            else
            {
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

            printf("\nRound: %d", round);
            printf("\nPlayer: %s\n", nameCurrentPlayer);
            printf("Enter a position: ");
            scanf("%d", &positionPlay);

            if (positionPlay < 1 || positionPlay > 9)
            {
                errorMessage[0] = '\x70';
                errorMessage[1] = '\x6F';
                errorMessage[2] = '\x73';
                errorMessage[3] = '\x69';
                errorMessage[4] = '\x63';
                errorMessage[5] = '\x61';
                errorMessage[6] = '\x6F';
                continue;
            }

            linePlay = posicoes[positionPlay - 1][0];
            colPlay = posicoes[positionPlay - 1][1];

            if (board[linePlay][colPlay] != '-')
            {
                errorMessage[0] = '\x70';
                errorMessage[1] = '\x6F';
                errorMessage[2] = '\x73';
                errorMessage[3] = '\x69';
                errorMessage[4] = '\x63';
                errorMessage[5] = '\x61';
                errorMessage[6] = '\x6F';
                errorMessage[7] = '\x20';
                errorMessage[8] = '\x6A';
                errorMessage[9] = '\x61';
                errorMessage[10] = '\x20';
                errorMessage[11] = '\x65';
                errorMessage[12] = '\x6D';
                errorMessage[13] = '\x20';
                errorMessage[14] = '\x75';
                errorMessage[15] = '\x73';
                errorMessage[16] = '\x6F';
                continue;
            }

            if (playerTurn == 1)
            {
                board[linePlay][colPlay] = 'X';
                playerTurn = 2;
            }
            else
            {
                board[linePlay][colPlay] = 'O';
                playerTurn = 1;
            }

            for (iterator = 0; iterator < 3; iterator++)
            {
                if (
                    (
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-') ||
                    ((board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'))
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-') ||
                ((board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'))
            {
                gameStatus = 0;
            }

            round++;
        }

        showBoard(board, (char *)&errorMessage);

        printf("\nOther player '%s' win!", nameCurrentPlayer);

        printf("\nEnd of the game!\n");
        printf("\nDo you want to start a game?");
        printf("\n1 - Yes");
        printf("\n2 - No");
        printf("\nChoose an option and press ENTER: ");

        scanf("%d", &optionRestart);

        switch (optionRestart)
        {
        case 1:
            gameStatus = 1;
            break;
        case 2:
            startScreen();
            break;
        }
    }
}

void startScreen()
{
    int option= 0;

    flashScreen();

    signal(SIGINT, catch_ctrl_c_and_exit);

    while (option < 1 || option> 3)
    {
        printf("Welcome to Tic Tac Toe");
        printf("\n1 - Play locally");
        printf("\n2 - Play online");
        printf("\n3 - About");
        printf("\n4 - Exit");
        printf("\nChoose an option and press ENTER: ");

        scanf("%d", &option);

        switch (option)
        {
        case 1:
            flashScreen();
            option= 0;
            game();
            break;
        case 2:
            flashScreen();
            option= 0;
            exit(conectGame());
            break;
        case 3:
            flashScreen();
            option= 0;
            printf("About the game!\n");
            break;
        case 4:
            flashScreen();
            option= 0;
            printf("You leave!\n");
            exit(0);
            break;
        default:
            flashScreen();
            option= 0;
            printf("Invalid option!\n");
            continue;
            break;
        }
    }
}

int main(int argc, char **argv)
{
    startScreen();

    return 0;
}
