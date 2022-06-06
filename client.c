#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "protocol.h"

#define HOSTPORTNUM 27429 /* Local port number of the client machine that peers will use to connect  */
#define PEERPORTNUM 27429 /* Peer(Opponent) port number */
#define STRLEN 1024
char name[STRLEN],    /* name used to join the server  */
    opponent[STRLEN]; /* name of the peer(Opponent)  */
pthread_t tid;        /*Thread ID*/
int yours = 0;        /* User's score*/
int oppos = 0;        /* Peer's(Opponent) Score*/
int ingame, turn;     /* Some variables to handle game logic. */

void *serverthread(void *parm);                    /* Thread function to handle communication with the server */
void *peerthread(char *ip);                        /* Thread function used when we want to open a connection with a peer */
int serversocket, peersocket, mysocket, consocket; /* All socket descriptors are stored as global variables */

/* Some variables to handle game logic. */
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
    printf("1|- Start game\n");
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
    if (strcmp(res.message, "HOLD") == 0)
    {
        memset(message, '\0', (strlen(message) + 1));
        strcat(message, "WAIT|");
        strcat(message, user);
        strcat(message, "|");

        return 1;
    }
    if (strcmp(res.message, "WAIT TIMEOUT") == 0)
    {
        authenticated_menu();
        return 1;
    }
    if (strstr(res.message, "invite"))
    {
        char *p;
        p = strtok(res.message, "|");
        p = strtok(NULL, "|");
        printf("%s", p);
        pthread_create(&tid, NULL, (void *)peerthread, p);
        pthread_exit(0); /* Exit this thread. */

        return 1;
    }
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
    /* The connection is established. Handle communication in a separate thread. */
    pthread_create(&tid, NULL, serverthread, (void *)&serversocket);

    /* For Invitation */
    /* Bind to a socket and start listening for incomming connections. */

    struct sockaddr_in peer;
    struct sockaddr_in serv;
    socklen_t socksize = sizeof(struct sockaddr_in);
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(HOSTPORTNUM);
    mysocket = socket(AF_INET, SOCK_STREAM, 0); /* Use a TCP connection */
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
    listen(mysocket, 1); /* Only accept one connection */
    consocket = accept(mysocket, (struct sockaddr *)&peer, &socksize);

    while (consocket)
    {
        pthread_cancel(tid);
        recv(consocket, &res, sizeof(res), 0);
        printf("[%d]-%s\n", res.status, res.message);
        strcpy(opponent, res.message);
        printf("\n%s has challenged you. Accept challenge? (y/n) ", opponent);
         
        /* Ask for approval */
        response r;
        scanf("%s", r.message);
        if (strcmp(r.message, "y") == 0)
        {
            yours = 0;
            oppos = 0;
            printf("\nYour Score: %d", yours);
            printf("\nOpponent's Score: %d", oppos);
            ingame = 1;
            send(consocket, &r, sizeof(r), 0);
        }
        else
        {
            yours = 0;
            oppos = 0;
            send(consocket, &r, sizeof(r), 0);
            printf("\nYou declined...\n");
            ingame = 0;
        }
    }

    close(mysocket);
    close(consocket);
    close(peersocket);
    close(serversocket);
    return 0;
}

void *serverthread(void *obj)
{

    printf("Server thread\n");
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
            close(mysocket);
            close(consocket);
            close(peersocket);
            close(serversocket);
            exit(1);
            break;
        }
    }
}

void *peerthread(char *ip)
{
    char buffer[1024];
    int len;
    response res;
    /* Open connection with the IP address passed as arguement. */
    struct sockaddr_in dest;
    peersocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(inet_network(ip));
    dest.sin_port = htons(PEERPORTNUM);
    printf("\nSending invite...\n");
    connect(peersocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));

    /* Send your name. */
    send(peersocket, name, strlen(name), 0);
    /* Receive acknowledgement from peer. */
    len = recv(peersocket, &res, sizeof(res), 0);

    if (strcmp(res.message, "y") == 0)
    {
        yours = 0;
        oppos = 0;
        printf("\nYour Score: %d", yours);
        printf("\nOpponent's Score: %d", oppos);
        ingame = 1;
    }
    else
    {
        yours = 0;
        oppos = 0;
        ingame = 0;
        printf("\n%s declined...\n", opponent);
    }
}
