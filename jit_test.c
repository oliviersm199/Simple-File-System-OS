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
    int filesize = (int)1024*5.5;
    char hello[filesize];
    printf("Before\n");
    printf("%s",hello); 
    int write = sfs_fwrite(ptr,hello,filesize);
    printf("Write:%d\n",write);
    char  buffer[filesize];
    int read = sfs_fread(ptr,buffer,filesize);
    printf("After\n");
    fwrite(buffer, 1, sizeof(buffer), stdout);
    printf("ASDFASDF%d\n",sfs_remove("new_file.txt"));
    
    return 0; 
}
