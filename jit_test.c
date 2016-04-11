
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
    int vals[2];
    vals[0] = sfs_fopen("myname.txt");
    int tmp = sfs_fopen("myname.txt");
    if(vals[0]==vals[1]){
	printf("Hurray\n");
    }else{
	printf("Woops\n");
    }

}
