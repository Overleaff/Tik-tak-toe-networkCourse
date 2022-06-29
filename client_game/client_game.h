void joinRoom(char buffer[]);
void makeMove(int move, char buffer[]);

int digit_check(char key[])
{
    for (int i = 0; i < strlen(key); i++)
    {
        if (isdigit(key[i]) == 0)
        {
            return 0;
        }
    }
    return 1;
}

/*
    Join room by id
    @param buffer: input_buffer
    @return
     */
void joinRoom(char buffer[])
{
    char roomId[13];

    do
    {
        printf("Enter room id:");

        scanf("%s", roomId);
        if (!digit_check(roomId))
        {
            printf("Enter an integer pls");
        }
    } while (!digit_check(roomId));
    // printf("Number is accepted, input finished!\n");

    strcat(buffer, "|");
    strcat(buffer, roomId);
}

/*
    Format moves message
    @param move: position play
    @param buffer: input_buffer
    @return
     */
void makeMove(int move, char buffer[])
{
    sprintf(buffer, "PLAY|%i\n", move);
}