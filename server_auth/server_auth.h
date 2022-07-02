void handleGuest(char name[], client_t *cli, char buffer[]);
void handleLogin(int *isLogin, client_t *cli, char buffer[]);
void handleReg(client_t *cli, char buffer[]);
void handleLogOut(int *isLogin, client_t *cli, char buffer[]);

/*
Handle request join as quest
@param name: handleGuest_name
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleGuest(char name[], client_t *cli, char buffer[])
{
    char *p = strtok(buffer, "|");
    strcpy(name, strtok(NULL, "|"));
    strcpy(cli->userInfo.name, name);
    bzero(buffer, BUFFER_SZ);
    strcpy(buffer, "MENU|");
    strcat(buffer, "ok");
    send_message(buffer, cli->uid);
}

/*
Handle login request
  @param isLogin : status login(1=yes)
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleLogin(int *isLogin, client_t *cli, char buffer[])
{
    char user[100];
    char pass[100];
    if (*isLogin == 1)
    {
        bzero(buffer, BUFFER_SZ);
        strcpy(buffer, " LGIN_ALR|");
        strcat(buffer, "Already login.Logged out first\n");
        // sprintf(buffer, "Already login.Logged out first\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        int fl = 1;
        userNode *n;
        char *p = strtok(buffer, "|");
        strcpy(user, strtok(NULL, "|"));
        printf("userName received :%s\n", user);
        strcpy(pass, strtok(NULL, "|"));
        printf("password received :%s\n", pass);

        pthread_mutex_lock(&auth_mutex);
        for (n = root2; n != NULL; n = n->next)
        {
            if (strcmp(user, n->element.name) == 0 && strcmp(pass, n->element.pass) == 0)
            {
                if (n->element.status == 0)
                {
                    // TODO: them 1 ham *isLogin = status (0 la chua log)
                    *isLogin = 1;
                    n->element.status = 1;
                    cli->userInfo.elo = n->element.elo;
                    strcpy(cli->username, cli->userInfo.name);
                    strcpy(cli->userInfo.name, user);
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
        pthread_mutex_unlock(&auth_mutex);

        if (fl == 0)
        {
            traversingList2(root2);
            bzero(buffer, BUFFER_SZ);
            strcpy(buffer, "LGIN_SUCC|");
            strcat(buffer, user);
            send_message(buffer, cli->uid); // login succes
        }
        else if (fl == -1)
        {

            bzero(buffer, BUFFER_SZ);
            strcpy(buffer, "LGIN_ALR|");
            strcat(buffer, "Already login\n");
            // sprintf(buffer, "Already login\n");
            send_message(buffer, cli->uid);
        }
        else
        {
            bzero(buffer, BUFFER_SZ);
            strcpy(buffer, "LGIN_FAIL|");
            strcat(buffer, "Login failure\n");
            // sprintf(buffer, "Login failure\n");
            send_message(buffer, cli->uid);
        }
     
    }
}

/*
Handle register request
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleReg(client_t *cli, char buffer[])
{
    char user[100];
    char pass[100];
 
    char *p = strtok(buffer, "|");
    strcpy(user, strtok(NULL, "|"));
    printf("userName register received :%s\n", user);
    strcpy(pass, strtok(NULL, "|"));
    printf("password register received :%s\n", pass);
    encrypt(pass, KEY);
    int fl = 1;
    userNode *n;

    pthread_mutex_lock(&reg_mutex);
    for (n = root2; n != NULL; n = n->next)
    {
        if (strcmp(user, n->element.name) == 0)
        {
            fl = 0;
            break;
        }
    }
    pthread_mutex_unlock(&reg_mutex);
    if (fl == 0)
    {
        bzero(buffer, BUFFER_SZ);
        strcpy(buffer, "REG_FAIL|");
        strcat(buffer, "Name exist.Register fail.\n");
        // sprintf(buffer, "Name exist.Register fail.\n");
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
        sprintf(tmp, "%d\n", ele.status);
        strcat(str, tmp);
        append(str);

        insertAtHead2(ele);
        traversingList2(root2);
        bzero(buffer, BUFFER_SZ);
        strcpy(buffer, "REG_SUCC|");
        strcat(buffer, "Register Successful\n");
        // sprintf(buffer, "Register Successful\n");
        send_message(buffer, cli->uid);
    }
    
}
/*
Handle logout request
@param isLogin : status login(1=yes)
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleLogOut(int *isLogin, client_t *cli, char buffer[])
{
    if (*isLogin == 0)
    {
        bzero(buffer, BUFFER_SZ);
        strcpy(buffer, "LOGOUT_FAIL|");
        strcat(buffer, "You need to login first\n");
        // sprintf(buffer, "Login First\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        char *p = strtok(buffer, "|");
        char us[100];
        strcpy(us, strtok(NULL, "|"));

        *isLogin = 0;
        cli->userInfo.elo = 1200; /*cap nhat elo client*/
        strcpy(cli->userInfo.name, cli->username);
        userNode *n;
        for (n = root2; n != NULL; n = n->next)
        {
            if (strcmp(us, n->element.name) == 0)
            {
                n->element.status = 0;
                // displayNode2(n);
                saveData1(n->element);
                // TODO : luu data vaofile khi logout
                // saveData("database.txt",n->element);
            }
        }
        traversingList2(root2);
        bzero(buffer, BUFFER_SZ);
        strcpy(buffer, "LOGOUT_SUCC|");
        strcat(buffer, "Logout Successful\n");
        // sprintf(buffer, "Logout Successful\n");
        send_message(buffer, cli->uid);
    }
}