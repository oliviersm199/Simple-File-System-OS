
/* sfs_test.c 
 * 
 * Written by Robert Vincent for Programming Assignment #1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sfs_api.h"

int main() {
    mksfs(1);
    int sfs_open = sfs_fopen("new_file.txt");
    int blocksize = 14*BLOCK_SZ;
    char buffer[blocksize];
    printf("Writing %d bytes\n",blocksize);
    for(int i = 0 ; i<blocksize;i++){
	buffer[i]='A' + (random() %26); 
    }
    int result = sfs_fwrite(sfs_open,buffer,blocksize);
    printf("Result:%d\n",result);
    return 0;
}
