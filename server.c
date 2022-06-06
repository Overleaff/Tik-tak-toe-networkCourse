
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

int checkRecvMsg(char buffer[1000], int newSocket, char *ip)
{
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
                    strcpy(p->element.ip, ip);
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
            strcpy(ele.ip, ip);
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
        char us[100];
        strcpy(us, strtok(NULL, "|"));
        char *ans = getNumberActive(us);
        res.status = 200;
        if (ans == NULL)
        {
            strcpy(res.message, "HOLD");
        }
        else
        {
            char str[100] = "invite|";
            strcat(str, ans);
            strcat(str, "|");
            strcpy(res.message, str);
        }

        byte_sent = send(newSocket, &res, sizeof(res), 0);

        return Check_Send(byte_sent);
    }
    if (strcmp(p, "WAIT") == 0)
    {

        sleep(5);

        char us[100];
        strcpy(us, strtok(NULL, "|"));
        char *ans = getNumberActive(us);
        res.status = 200;
        if (ans == NULL)
        {
            strcpy(res.message, "WAIT TIMEOUT");
        }
        else
        {
            char str[100] = "invite|";
            strcat(str, ans);
            strcat(str, "|");
            strcpy(res.message, str);
        }

        byte_sent = send(newSocket, &res, sizeof(res), 0);

        return Check_Send(byte_sent);
    }
    // wait for other to log in to start a game
    return 0;
}

void *serverthread(void *client_socket)
{

    int check_status = 1;
    char buffer[1024];
    int newSocket = *(int *)client_socket; /*Thread Socket descriptor*/
    printf("Thread ID:%d\n", newSocket);
    traversingList(root);
    /* Get client's IP address*/
    char ip[INET_ADDRSTRLEN]; /*Char array to store client's IP address*/
    memset(ip, '\0', (strlen(ip) + 1));
    struct sockaddr_in peeraddr;
    socklen_t peeraddrlen = sizeof(peeraddr);
    getpeername(newSocket, &peeraddr, &peeraddrlen); /*Retrives address of the peer to which a socket is connected*/
    // inet_ntop(AF_INET, &(peeraddr.sin_addr), ip, INET_ADDRSTRLEN); /*Binary to text string*/ /*Retriving IP addrees of client and converting
    // it to text and storing it in IP char array*/

    // mark
    strcpy(ip, inet_ntoa(peeraddr.sin_addr));
    printf("%s\n", ip);
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

        if (checkRecvMsg(buffer, newSocket, ip) <= 0)
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
        if (pthread_create(&tid, NULL, serverthread, &newSocket) < 0)
        {
            printf("Cannot create thread");
            break;
        }
    }
    printf("End server");
    close(sockfd);

    return 0;
}
