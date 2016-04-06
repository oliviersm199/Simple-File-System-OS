
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
    int result = sfs_fopen("friends.txt");    
    printf("result:%d\n",result);
    int result2 = sfs_getfilesize("friends.txt");
    printf("File Size is: %d",result2);
    int result3 = sfs_getfilesize("friends2.txt");
    printf("File Size is: %d",result3);
    return 0; 
}
