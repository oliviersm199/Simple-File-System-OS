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
    int ptr = sfs_fopen("new_file.txt");
    printf("File Open:%d\n",ptr);
    char * hello = "HELLO WORLD I AM FINE AND YOU?";
    int write = sfs_fwrite(ptr,hello,8);
    printf("Write:%d\n",write);
    char  buffer[1024];
    memset(buffer, '~', sizeof(buffer));
    int read = sfs_fread(ptr,buffer,strlen(hello));
    printf("%d\n",read);
    printf("%s\n",buffer);
    return 0; 
}
