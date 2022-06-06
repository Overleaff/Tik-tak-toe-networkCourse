#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "protocol.h"

char message[1000];
char user[100] = "";
response res;
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
    printf("1|- Start game.\n");
    printf("2|- Logout.\n");
    printf("3|- Exit.\n");
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
    return 0;
}
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server;

    int byte_sent, byte_received;
    char *SERV_IP;
    short SERV_PORT;

    if (argc != 3)
    {
        printf("Usage: %s <IPAddress> <PortNumber> ", argv[0]);
        exit(1);
    }
    SERV_IP = argv[1];
    SERV_PORT = atoi(argv[2]);

    // Step 1: Construct socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error: ");
        return 0;
    }
    // Step 2: Define the address of the server
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERV_PORT);
    server.sin_addr.s_addr = inet_addr(SERV_IP);

    printf("CLIENT START\n");

    int ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (ret < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    while (1)
    {
        byte_received = recv(sockfd, &res, sizeof(res), 0);
        if (byte_received == -1)
        {
            printf("\nError!Cannot receive data from sever!\n");
            close(sockfd);
            exit(-1);
        }
        else
            printf("[%d]-%s\n", res.status, res.message);

        if (check_message(res) == 1)
        {
            byte_sent = send(sockfd, message, strlen(message), 0);
            if (byte_sent <= 0)
            {
                printf("\nError!Cannot send data to sever!\n");
                close(sockfd);
                exit(-1);
            }
        }
        else
            break;
    }
    close(sockfd);
    return 0;
}


