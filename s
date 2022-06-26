//login
if (isLogin == 1)
                        {
                            bzero(buffer, BUFFER_SZ);
                            strcpy(buffer, " LGIN_ALR|");
                            strcat(buffer, "Already login.Logged out first\n");
                            //sprintf(buffer, "Already login.Logged out first\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            int fl = 1;
                            userNode *n;
                            p = strtok(buffer, "|");
                            strcpy(user, strtok(NULL, "|"));
                            printf("userName received :%s\n", user);
                            strcpy(pass, strtok(NULL, "|"));
                            printf("password received :%s\n", pass);
                            for (n = root2; n != NULL; n = n->next)
                            {
                                if (strcmp(user, n->element.name) == 0 && strcmp(pass, n->element.pass) == 0)
                                {
                                    if (n->element.status == 0)
                                    {
                                        // TODO: them 1 ham isLogin = status (0 la chua log)
                                        isLogin = 1;
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

                            if (fl == 0)
                            {
                                traversingList2(root2);
                                bzero(buffer, BUFFER_SZ);
                                strcpy(buffer,"LGIN_SUCC|");
                                strcat(buffer, user);
                                send_message(buffer, cli->uid); //login succes
                            }
                            else if (fl == -1)
                            {

                                bzero(buffer, BUFFER_SZ);
                                strcpy(buffer, "LGIN_ALR|");
                                strcat(buffer, "Already login\n");
                                //sprintf(buffer, "Already login\n");
                                send_message(buffer, cli->uid);
                            }
                            else
                            {
                                bzero(buffer, BUFFER_SZ);
                                strcpy(buffer, "LGIN_FAIL|");
                                strcat(buffer, "Login failure\n");
                                //sprintf(buffer, "Login failure\n");
                                send_message(buffer, cli->uid);
                            }
                        }
// create part

// logout
 if (isLogin == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            strcpy(buffer, "LOGOUT_FAIL|");
                            strcat(buffer, "You need to login first\n");
                            //sprintf(buffer, "Login First\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            p = strtok(buffer, "|");
                            char us[100];
                            strcpy(us, strtok(NULL, "|"));

                            isLogin = 0;
                            cli->userInfo.elo = 0; /*cap nhat elo client*/
                            
                            strcpy(cli->userInfo.name, cli->username);
                            userNode *n;
                            for (n = root2; n != NULL; n = n->next)
                            {
                                if (strcmp(us, n->element.name) == 0)
                                {
                                    n->element.status = 0;
                                    //displayNode2(n);
                                    saveData1(n->element);
                                    // TODO : luu data vaofile khi logout
                                    // saveData("database.txt",n->element);
                                }
                            }
                            traversingList2(root2);
                            bzero(buffer, BUFFER_SZ);
                            strcpy(buffer, "LOGOUT_SUCC|");
                            strcat(buffer, "Logout Successful\n");
                            //sprintf(buffer, "Logout Successful\n");
                            send_message(buffer, cli->uid);
                            // TODO:save data vao file database
                        } 

if (strcmp(buffer, "CREATE RANK") == 0 && isLogin == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            strcpy(buffer, "ROOM_FAIL|");
                            strcat(buffer, "You need to login\n");
                            //sprintf(buffer, "You need to login\n");
                            send_message(buffer, cli->uid);
                        }
                        else
                        {
                            flag = 0;

                            pthread_mutex_lock(&rooms_mutex);

                            for (int i = 0; i < MAX_ROOMS; i++)
                            {
                                if (rooms[i])
                                {
                                    if (rooms[i]->player1->uid == cli->uid)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        strcpy(buffer, "ROOM_ALR|");
                                        strcat(buffer, "you are already in the room\n");
                                        //sprintf(buffer, "you are already in the room\n");
                                        send_message(buffer, cli->uid);
                                        flag = 1;
                                        break;
                                    }

                                    if (rooms[i]->player2 != 0)
                                    {
                                        if (rooms[i]->player2->uid == cli->uid)
                                        {
                                            bzero(buffer, BUFFER_SZ);
                                            strcpy(buffer, "ROOM_ALR|");
                                            strcat(buffer, "you are already in the room\n");
                                            //sprintf(buffer, "you are already in the room\n");
                                            send_message(buffer, cli->uid);
                                            flag = 1;
                                            break;
                                        }
                                    }
                                }
                            }

                            pthread_mutex_unlock(&rooms_mutex);

                            if (flag != 1)
                            {
                                // clients settings
                                room_t *room = (room_t *)malloc(sizeof(room_t));
                                room->player1 = cli;
                                room->player2 = 0;
                                room->uid = roomUid;
                                printf("Roomid create:%d\n", room->uid);
                                if (strcmp(buffer, "CREATE") == 0)
                                {
                                    strcpy(room->roomType, "[NORMAL] ");
                                }
                                else
                                    strcpy(room->roomType, "[RANK] ");
                                strcpy(room->state, room->roomType);
                                strcat(room->state, "waiting for second player");

                                // add room to queue
                                queue_add_room(room);
                                bzero(buffer, BUFFER_SZ);
                                bzero(tmp, BUFFER_SZ);
                                strcpy(buffer, "ROOM_SUCC|");
                                sprintf(tmp, "you created a new room number %i\n", roomUid);
                                strcat(buffer, tmp);
                                send_message(buffer, cli->uid);
                                roomUid++;
                            }
                        }

                         int researched = 0;
                        int already = 0;
                        pthread_mutex_lock(&rooms_mutex);

                        for (int j = 0; j < MAX_ROOMS; j++)
                        {
                            if (rooms[j])
                            {  
                                if (rooms[j]->player1->uid == cli->uid)
                                { 

                                    // checkLogin
                                    if (strstr(rooms[j]->state, "[RANK] ") && isLogin == 0)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                        strcpy(buffer, "JOIN_FAIL|");
                                        strcat(buffer, "You need to login\n");
                                        send_message(buffer, cli->uid);
                                        break;
                                    }

                                    already = 1;

                                    bzero(buffer, BUFFER_SZ);
                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "JOIN_ALR|");
                                    sprintf(tmp, "you are already in the room number: %i\n", rooms[j]->uid);
                                    strcat(buffer, tmp);
                                   
                                    send_message(buffer, cli->uid);
                                    break;
                                    
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);

                        if (already == 1)
                        {
                            continue;
                        }

                        pthread_mutex_lock(&rooms_mutex);

                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                if (rooms[i]->uid == number)
                                {
                                     
                                     
                                    researched = 1;

                                    if (rooms[i]->player2 != 0)
                                    {
                                       

                                            if (rooms[i]->player2->uid == cli->uid)
                                            {
                                                bzero(buffer, BUFFER_SZ);
                                                
                                                strcpy(buffer, "ROOM_ALR|");
                                                
                                                strcat(buffer, "you are already in the room\n");
                                                send_message(buffer, cli->uid);
                                                break;
                                            }

                                           
                                            bzero(buffer, BUFFER_SZ);
                                            bzero(tmp, BUFFER_SZ);
                                            strcpy(buffer, "ROOM_FULL|");
                                            sprintf(tmp, "room number: %i, is already full\n", rooms[i]->uid);
                                            strcat(buffer, tmp);
                                           // sprintf(buffer, "room number: %i, is already full\n", rooms[i]->uid);
                                            send_message(buffer, cli->uid);
                                            break;
                                     }
                                     
                                    rooms[i]->player2 = cli;
                                    if (strstr(rooms[i]->state, "[RANK] ") && isLogin==0)
                                    {
                                        rooms[i]->player2 = 0;
                                        printf("Xin chao %d", isLogin);
                                        bzero(buffer, BUFFER_SZ);
                                        strcpy(buffer, "JOIN_FAIL|");
                                        strcat(buffer, "You need to login\n");
                                        send_message(buffer, cli->uid);
                                        break;
                                    }else{
                                    strcpy(rooms[i]->state, rooms[i]->roomType);
                                    strcat(rooms[i]->state, "waiting start");
                                    bzero(buffer, BUFFER_SZ);
                                    printf("%s enter the room number: %i\n", cli->userInfo.name, number);

                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "JOIN_SUCC|");
                                    sprintf(tmp, "'%s' entered your room\n", cli->userInfo.name);
                                    strcat(buffer, tmp);
                                    //sprintf(buffer, "'%s' entered your room\n", cli->userInfo.name);
                                    send_message(buffer, rooms[i]->player1->uid);

                                    bzero(buffer, BUFFER_SZ);
                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "JOIN_SUCC|");
                                    sprintf(tmp, "you has entered the room number: %i\n", number);
                                    strcat(buffer, tmp);
                                   // sprintf(buffer, "you has entered the room number: %i\n", number);
                                    send_message(buffer, cli->uid);
                                    break;
                                    }
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);

                        if (researched == 0)
                        {
                            bzero(buffer, BUFFER_SZ);
                            bzero(tmp, BUFFER_SZ);
                            strcpy(buffer, "ROOM_NOT_FOUND|");
                            sprintf(tmp, "could not find the room number %i\n", number);
                            strcat(buffer, tmp);
                            //sprintf(buffer, "could not find the room number %i\n", number);
                            send_message(buffer, cli->uid);
                        }

                        pthread_mutex_lock(&rooms_mutex);

                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                char *list = (char *)malloc(BUFFER_SZ * sizeof(char));

                                if (rooms[i]->player2 != 0)
                                {
                                    sprintf(list, "%i)\n    room state: %s  \n    player1: %s - elo: %d\n    player2: %s - elo: %d\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->userInfo.name, rooms[i]->player1->userInfo.elo
                                    , rooms[i]->player2->userInfo.name, rooms[i]->player2->userInfo.elo);
                                }
                                else
                                {
                                    sprintf(list, "%i)\n    room state: %s \n    player1: %s - elo: %d\n", rooms[i]->uid, rooms[i]->state, rooms[i]->player1->userInfo.name, rooms[i]->player1->userInfo.elo);
                                }

                                bzero(buffer, BUFFER_SZ);
                                
                                strcpy(buffer, "ROOM_LISTS|");
                               
                                strcat(buffer, list);
                                send_message(buffer ,cli->uid);
                                free(list);
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);




                        int remove_room = 0;
                        int room_number = 0;

                        pthread_mutex_lock(&rooms_mutex);
                        printf("HEllo\n");
                        for (int i = 0; i < MAX_ROOMS; i++)
                        {
                            if (rooms[i])
                            {
                                if (rooms[i]->player1->uid == cli->uid)
                                {
                                    if (rooms[i]->player2 != 0)
                                    {
                                        bzero(buffer, BUFFER_SZ);
                                   
                                        bzero(tmp, BUFFER_SZ);
                                        strcpy(buffer, "LEAVE_ROOM|");
                                        sprintf(tmp, "%s left the room, now you are the owner\n", rooms[i]->player1->userInfo.name);
                                        strcat(buffer, tmp);
                                        //sprintf(buffer, "%s left the room, now you are the owner\n", rooms[i]->player1->userInfo.name);
                                        send_message(buffer, rooms[i]->player2->uid);

                                        rooms[i]->player1 = rooms[i]->player2;
                                        rooms[i]->player2 = 0;
                                        strcpy(rooms[i]->state, rooms[i]->roomType);
                                        strcat(rooms[i]->state, "waiting for second player");
                                    }
                                    else
                                    { // ko co ai
                                    // TODO : xoa room 1 trc roi ms xoa dc room 2
                                        remove_room = 1;
                                        room_number = rooms[i]->uid;
                                    }

                                    bzero(buffer, BUFFER_SZ);
                                 

                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "LEAVE_ROOM|");
                                    sprintf(tmp, "you left the room %i\n", rooms[i]->uid);
                                    strcat(buffer, tmp);
                                    //sprintf(buffer, "you left the room %i\n", rooms[i]->uid);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                                else if (rooms[i]->player2->uid == cli->uid)
                                {
                                    bzero(buffer, BUFFER_SZ);

                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "LEAVE_ROOM|");
                                    sprintf(tmp, "%s left the room\n", rooms[i]->player2->userInfo.name);
                                    strcat(buffer, tmp);

                                   // sprintf(buffer, "%s left the room\n", rooms[i]->player2->userInfo.name);
                                    send_message(buffer, rooms[i]->player1->uid);

                                    rooms[i]->player2 = 0;
                                    strcpy(rooms[i]->state, rooms[i]->roomType);
                                    strcat(rooms[i]->state, "waiting for second player");

                                    bzero(buffer, BUFFER_SZ);
                                    bzero(tmp, BUFFER_SZ);
                                    strcpy(buffer, "LEAVE_ROOM|");
                                    sprintf(tmp, "you left the room %i\n", rooms[i]->uid);
                                    strcat(buffer, tmp);
                                    //sprintf(buffer, "you left the room %i\n", rooms[i]->uid);
                                    send_message(buffer, cli->uid);
                                    break;
                                }
                            }
                        }

                        pthread_mutex_unlock(&rooms_mutex);
                        printf("HEllo1235\n");
                        if (remove_room == 1)
                        {
                            printf("Room delete:%d\n", room_number);
                            queue_remove_room(room_number);
                            roomUid++; //OR  roomUid --
                        }




                         /* GUEST
                   p = strtok(buffer, "|");
                   strcpy(name, strtok(NULL, "|"));
                   strcpy(cli->userInfo.name, name);
                   bzero(buffer, BUFFER_SZ);
                   strcpy(buffer, "MENU|");
                   strcat(buffer, "ok");
                   send_message(buffer, cli->uid);*/