
void encrypt(char password[],int key){
    for(int i=0;i<strlen(password);i++){
        password[i] = password[i] - key;
    }
}

void decrypt(char password[], int key)
{
    for (int i = 0; i < strlen(password); i++)
    {
        password[i] = password[i] + key;
    }
}

