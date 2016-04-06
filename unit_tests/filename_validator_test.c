#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int validate_filename(char * name){
    int str_len = strlen(name);
    if(!str_len){
        return 0;
    }

    //verifying dot position
    char * dot = strchr(name, '.');

    //scenario if dot exists
    if(dot == NULL){
        if(str_len<=20) return 1;
        return 0;
    }
    //getting index by calculating offset of dot ptr and char ptr.
    int index = dot - name;
    int count = 0;

    while(count+index<str_len){
        count++;
    }

    //checking if characters including dot are proper length
    if(count > 4){
        return 0;
    }

    //checking if strlen is proper.
    if(strlen(name)-count>16){
        return 0;
    }
    return 1;
}
//EXPECTED OUTPUT: 1,1,1,0,1,1,0
int main(){
    char * filename = "filename";
    printf("Trial 1: %d\n",validate_filename(filename));


    filename = "filename.txt";
    printf("Trial 2: %d\n",validate_filename(filename));

    //
    filename = "filenamenamenamename";
    printf("Trial 3: %d\n",validate_filename(filename));


    filename = "filenamenamename.txt";
    printf("Trial 4: %d\n",validate_filename(filename));


    filename = "filenamenamenamee.txt";
    printf("Trial 5: %d\n",validate_filename(filename));

    filename = "file";
    printf("Trial6:%d\n",validate_filename(filename));

    filename = "f.txt";
    printf("Trial7:%d\n",validate_filename(filename));

    filename="";
    printf("Trial8:%d\n",validate_filename(filename));


    return 1;
}

