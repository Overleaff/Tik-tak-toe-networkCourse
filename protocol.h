
// LOGIN username pass
// 200 - Login
typedef struct login
{
    char user[100];
    char pass[100];
    int status; //1 online - 0 off
} userLogin;

typedef struct protocol1
{
    char command[100];
    userLogin info;
} loginRequest;

typedef struct protocol2
{
    int status;
    char message[1000];
} response;
