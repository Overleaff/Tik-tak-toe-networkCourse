#include <stdio.h>
#include <stdlib.h>
#include "singLL.h"
#include <string.h>
#include <ctype.h>
#define BUFFER_SIZE 1000
int main()
{

    //getLine2("database.txt","tu");
    elementtype e;
    strcpy(e.name,"nga");
    strcpy(e.pass,"s");
    e.status=1;
    e.elo=2;
    saveData1(e);
}
