
/* sfs_test.c 
 * 
 * Written by Robert Vincent for Programming Assignment #1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sfs_api.h"

int main() {
    mksfs(1);
    int result = sfs_fopen("friends1.txt");    
    int result2 = sfs_fopen("friends2.txt");
    int result3 = sfs_fopen("friends3.txt");
    int result4 = sfs_fopen("friends4.txt");
    int result5 = sfs_fopen("nihaoworld.txt");
    char * buffer = (char *)malloc(sizeof(20));
    
    while(sfs_getnextfilename(buffer)){
        printf("%s\n",buffer);
    }
   
    return 0; 
}
