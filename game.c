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

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[NAME_LEN];
int player = 1;

pthread_t lobby_thread;
pthread_t recv_msg_thread;
pthread_t multiplayer_game;

char *ip="127.0.0.1";
int port = 2050;

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
    int line;

    flashScreen();

    if (*errorMessage != '\x00')
    {
        printf("Warning: %s!\n\n", errorMessage);
        *errorMessage = '\x00';
    }

    printf("#############\n");

    for (line = 0; line < 3; line++)
    {
        printf("# %c | %c | %c #", board[line][0], board[line][1], board[line][2]);

        printf("\n");
    }

    printf("#############\n");

    showPositions();
}

void createBoard(char board[3][3])
{
    int line, col;

    for (line = 0; line < 3; line++)
    {
        for (col = 0; col < 3; col++)
        {
            board[line][col] = '-';
        }
    }
}


void menu();

void *lobby(void *arg);

void recv_msg_handler();

void *multiplayerGame(void *arg)
{
    int playerTurn;
    playerTurn = player;
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
    int positions[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

    char errorMessage[255] = {'\x00'};
    char *currentPlayerName;
    char message[BUFFER_SZ] = {};

    int receive = recv(sockfd, message, BUFFER_SZ, 0);

    if (receive > 0) {
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
                currentPlayerName = (char *)&namePlayer1;
            } 
            else
            {
                currentPlayerName = (char *)&namePlayer2;
            }

            showBoard(board, (char *)&errorMessage);

            printf("\nround: %d", round);
            printf("\nplayer: %s\n", currentPlayerName);

            while (valid_play == 0)
            {
                bzero(message, BUFFER_SZ);

                int receive = recv(sockfd, message, BUFFER_SZ, 0);

                if (receive > 0) {
                    valid_play = 1;

                    setbuf(stdin, 0);
                    setbuf(stdout, 0);

                    if (strcmp(message, "vez1\n") == 0)
                    {
                        printf("Digite uma posicao: ");
                        scanf("%d", &positionPlay);


                        linePlay = positions[positionPlay - 1][0];
                        colPlay = positions[positionPlay - 1][1];


                        if (valid_play == 1)
                        {
                            sprintf(message, "play %i\n", positionPlay);
                            send(sockfd, message, strlen(message), 0);
                            bzero(message, BUFFER_SZ);
                        }
                        
                    }
                    else if (strcmp(message, "vez2\n") == 0)
                    {
                        printf("O outro player esta jogando...\n");

                        played = 0;

                        while (played == 0)
                        {
                            int receive = recv(sockfd, message, BUFFER_SZ, 0);

                            if (receive > 0) {
                                sscanf(message, "%i", &numberPlayed);

                                linePlay = positions[numberPlayed - 1][0];
                                colPlay = positions[numberPlayed - 1][1];

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
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-'
                    )
                        ||
                    (
                        (board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'
                    )
                )
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-'
                )
                    ||
                (
                    (board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'
                )
            )
            {
                gameStatus = 0;
            }

            round++;
            valid_play = 0;
            bzero(message, BUFFER_SZ);
        }

        bzero(message, BUFFER_SZ);

        int receive = recv(sockfd, message, BUFFER_SZ, 0);

        if (receive > 0) {
            setbuf(stdin, 0);
            setbuf(stdout, 0);

            showBoard(board, (char *)&errorMessage);


            if (strcmp(message, "win1\n") == 0)
            {
                printf("\nO player '%s' won!", currentPlayerName);
            }
            else if (strcmp(message, "win2\n") == 0)
            {
                printf("\nO player '%s' won!", currentPlayerName);
            }

            printf("\nEnd of the game!\n");

            sleep(6);

            if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0) {
                printf("ERROR: pthread\n");
                return NULL;
            }

            if (pthread_create(&recv_msg_thread, NULL, (void*)recv_msg_handler, NULL) != 0) {
                printf("ERROR: pthread\n");
                return NULL;
            }

            pthread_detach(pthread_self());
            pthread_cancel(multiplayer_game);

        }
        
    }

    return NULL;
}

void *lobby(void *arg)
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

    return NULL;
}

void recv_msg_handler()
{
    char message[BUFFER_SZ] = {};

    flashScreen();

    while(1)
    {
        int receive = recv(sockfd, message, BUFFER_SZ, 0);

        if (receive > 0) {
            if (strcmp(message, "ok") == 0)
            {
                printf("Commands:\n");
                printf("\t -list\t\t\t List all tic-tac-toe rooms\n");
                printf("\t -create\t\t Create one tic-tac-toe room\n");
                printf("\t -join {room number}\t Join in one tic-tac-toe room\n");
                printf("\t -leave\t\t\t Back of the one tic-tac-toe room\n");
                printf("\t -start\t\t\t Starts one tic-tac-toe game\n\n");

                str_overwrite_stdout();
            }
            else if (strcmp(message, "start game\n") == 0)
            {
                pthread_cancel(lobby_thread);
                // pthread_kill(recv_msg_thread, SIGUSR1);   
                
                player = 1;
                if (pthread_create(&multiplayer_game, NULL, (void*)multiplayerGame, NULL) != 0) {
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
                if (pthread_create(&multiplayer_game, NULL, (void*)multiplayerGame, NULL) != 0) {
                    printf("ERROR: pthread\n");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);

                // pthread_kill(lobby_thread, SIGUSR1);   
            }
            else
            {
                printf("%s", message);
                str_overwrite_stdout();
            }
        } else if (receive == 0) {
            break;
        }

        bzero(message, BUFFER_SZ);
    }
}

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

int conectGame()
{
    setbuf(stdin, 0);

    printf("Enter your name: ");
    fgets(name, BUFFER_SZ, stdin);
    trim_lf(name, BUFFER_SZ);

    // strcpy(name, "murilo");

    if (strlen(name) > NAME_LEN - 1 || strlen(name) < 2) {
        printf("Enter name corretly\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
   
    //socket settings
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // connect to the server
    int err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // send the name
    send(sockfd, name, NAME_LEN, 0);

    if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0) {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&recv_msg_thread, NULL, (void*)recv_msg_handler, NULL) != 0) {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while(1)
    {
        if (flag) {
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
    int optionReinicio;
    int gaming = 1;

    char errorMessage[255] = {'\x00'};

    char namePlayer1[32];
    char namePlayer2[32];
    char *currentPlayerName;

    setbuf(stdin, 0);

    printf("Name do 1° player: ");
    fgets(namePlayer1, 32, stdin);

    namePlayer1[strlen(namePlayer1) - 1] = '\x00';

    printf("Name do 2° player: ");
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
                currentPlayerName = (char *)&namePlayer1;
            } 
            else
            {
                currentPlayerName = (char *)&namePlayer2;
            }

            int positions[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

            printf("\nround: %d", round);
            printf("\nplayer: %s\n", currentPlayerName);
            printf("Enter a position: ");
            scanf("%d", &positionPlay);

            if (positionPlay < 1 || positionPlay > 9) {
                errorMessage[0] = '\x70';
                errorMessage[1] = '\x6F';
                errorMessage[2] = '\x73';
                errorMessage[3] = '\x69';
                errorMessage[4] = '\x63';
                errorMessage[5] = '\x61';
                errorMessage[6] = '\x6F';
                continue;
            }

            linePlay = positions[positionPlay - 1][0];
            colPlay = positions[positionPlay - 1][1];

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
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-'
                    )
                        ||
                    (
                        (board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'
                    )
                )
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-'
                )
                    ||
                (
                    (board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'
                )
            )
            {
                gameStatus = 0;
            }

            round++;
        }

        showBoard(board, (char *)&errorMessage);

        printf("\nThe player '%s' has won!", currentPlayerName);

        printf("\nGame over!\n");
        printf("\nDo you want to restart the game?");
        printf("\n1 - Yes");
        printf("\n2 - No");
        printf("\nChoose an option and press ENTER: ");

        scanf("%d", &optionReinicio);


        switch (optionReinicio)
        {
            case 1:
                gameStatus = 1;
                break;
            case 2:
                menu();
                break;
        }
    }
}

void menu()
{
    int option = 0;

    flashScreen();

    signal(SIGINT, catch_ctrl_c_and_exit);

    while (option < 1 || option > 3)
    {
        printf("Welcome to the Game");
        printf("\n1 - Play Locally");
        printf("\n2 - Play Online");
        printf("\n3 - About");
        printf("\n4 - Exit");
        printf("\nChoose an option and press ENTER: ");

        scanf("%d", &option);

        switch (option)
        {
            case 1:
                flashScreen();
                option = 0;
                game();
                break;
            case 2:
                flashScreen();
                option = 0;
                exit(conectGame());
                break;
            case 3:
                flashScreen();
                option = 0;
                printf("About the game!\n");
                break;
            case 4:
                flashScreen();
                option = 0;
                printf("You leave!\n");
                exit(0);
                break;
            default:
                flashScreen();
                option = 0;
                printf("invalid option!\n");
                break;
        }
    }
}

int main(int argc, char **argv)
{   
    //ip = argv[1];
    //printf("%s",ip);
    menu();
    return 0;
}
