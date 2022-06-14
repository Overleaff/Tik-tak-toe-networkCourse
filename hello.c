#include <stdio.h>
#include <stdlib.h>
#include "singLL.h"
#include <string.h>
#include <ctype.h>
#define BUFFER_SIZE 1000
int main()
{

   elementtype t;
   strcpy(t.name,"tu");
   strcpy(t.pass,"s");
   t.elo=3;
   t.status=2;
   saveData("database.txt",t);
}
