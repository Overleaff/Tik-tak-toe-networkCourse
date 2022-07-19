void guest(char uname[], char buffer[]);
void login(char uname[], char pass[], char buffer[]);
void signUp(char uname[], char pass[], char buffer[]);
void logOut(char name[], char buffer[]);

/*
Join as guest
@param uname: guest_name
@param buffer: input_buffer
@return
 */
void guest(char uname[], char buffer[])
{
    printf("Enter name : ");
    scanf("%s", uname);
    memset(buffer, '\0', (strlen(buffer) + 1));
    strcpy(buffer, "GUEST|");
    strcat(buffer, uname);
}

/*
Log in account
@param name: name_logout
@param buffer: input_buffer
@return
 */
void login(char uname[], char pass[], char buffer[])
{
    printf("Enter userId : ");
    scanf("%s", uname);
    printf("Enter password : ");
    scanf("%s", pass);

    encrypt(pass, KEY);
    memset(buffer, '\0', (strlen(buffer) + 1));
    strcpy(buffer, "LOGIN|");
    strcat(buffer, uname);
    strcat(buffer, "|");
    strcat(buffer, pass);
}

/*
Signup account
@param uname: name_register
@param pass: pass_register
@param buffer: input_buffer
@return
 */
void signUp(char uname[], char pass[], char buffer[])
{
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

/*
Log out account
@param name: name_logout
@param buffer: input_buffer
@return
 */
void logOut(char name[], char buffer[])
{
    strcpy(buffer, "LOGOUT|");
    strcat(buffer, name);
    strcat(buffer, "|");
}