#include <stdio.h>
#include "singLL.h"

int main()
{
    elementtype tmp;

    int i;
    int n;
    importTextFile("database.txt");
    traversingList(root);
    printf("%s",getNumberActive("tu"));
    /*
    for (i = 0; i < 4; i++)
    {
        tmp = readNode();
        insertAtHead(tmp);
        displayNode(root);
    };
    printf("Info\n");
    traversingList(root);
    
    printf("test for reverse list.\n");
    root = list_reverse(root);
    traversingList(root);
  
    printf("Delete first node.\n");  //position start from 0
    deletefirstelement();
    traversingList(root);

    printf("Delete curent node.\n");
    deletecur();
    traversingList(root);
    
    printf("Enter the position.\n");
    scanf("%d", &n);
    printf("Delete at position\n");
    deleteAtposi(n);
    traversingList(root);

    printf("Enter the position.\n"); // 1 insert after 1;
    scanf("%d", &i);
    printf("Print at position\n");
    tmp = readNode();
    insertAtposi(tmp, i);
    traversingList(root);
     */
    freeList(root);
    return 0;
}
