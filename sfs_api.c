#include "disk_emu.h"
#include "sfs_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
superblock_t sb;

inode_table i_table;

bitmap bm;

root_directory root_dir;

fdt_table f_table;

void init_superblock(){
    sb.magic = 0xACBD0005;
    sb.block_size = BLOCK_SZ;
    sb.fs_size = NUM_BLOCKS * BLOCK_SZ;
    sb.inode_table_len = NUM_INODE_BLOCKS;
    sb.root_dir_inode = 0;
}

//bitmap functions
void init_bitmap(){
    int i;
    for(i=0;i<NUM_BLOCKS;i++){
        bm.index[i] = BLOCK_AVALIABLE;
    }
    bm.first_free_block = 0;
}
	
int occupy_block(){
    int return_value =  bm.first_free_block;    
    //updating the last_free_block parameter
    for(int i =bm.first_free_block+1;i<NUM_BLOCKS;i++){
	if(bm.index[i]==BLOCK_AVALIABLE){
            bm.first_free_block=i;
	    break;
	}
    }
    if(return_value == bm.first_free_block){
	printf("No room in memory\n");
	return -1; 
    }
    bm.index[return_value] = BLOCK_OCCUPIED;
    write_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
    return return_value; 
}

int release_block(int release){
    //block is free to overwrite.
    if(release>=0 && release<NUM_BLOCKS){
        if(bm.index[release]==BLOCK_OCCUPIED){
            bm.index[release] = BLOCK_AVALIABLE;
	    if(bm.first_free_block>release){
		bm.first_free_block=release;
	    }
	    write_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
	    return 0;
	}
	else{
	    printf("Releasing block that has not been allocated\n");
	    return -1;
	}
    }
    printf("Releasing block that is out of range\n");
    return -2;
}


//inode functionalities
void init_inode_table(){
    for(int i = 0 ; i<NUM_INODES;i++){
	i_table.index[i].inuse = INODE_FREE;
    }
    i_table.first_free_inode = 0;	
}

int occupy_inode(){
    int return_value = i_table.first_free_inode;
    for(int i =return_value+1;i<NUM_INODES;i++){
	if(i_table.index[i].inuse == INODE_FREE){
		i_table.first_free_inode = i;
		break;
	}
    }
    if(return_value == i_table.first_free_inode){
	printf("%d", i_table.first_free_inode);
	printf("No free inode\n");
        return -1;
    }
    i_table.index[return_value].inuse =INODE_IN_USE;
    write_blocks(1,sb.inode_table_len,&i_table);
    return return_value;
}

int release_inode(int release){
    if(release>=0 && release < NUM_INODES){
    if(i_table.index[release].inuse==INODE_IN_USE){
      	i_table.index[release].inuse = INODE_FREE;
        if(i_table.first_free_inode> release) i_table.first_free_inode = release;
	    return 0;    
    	}
        else{
            printf("Trying to release unused iNode\n");
            return -1;
        }
    }
    printf("Invalid index for inode");
    return -2;
}


int create_empty_inode(){
    int inode_index = occupy_inode();
    if(inode_index<0){
	return -1;
    }
    inode_t * newInode = &i_table.index[inode_index];
    newInode -> size = 0;
    write_blocks(1,sb.inode_table_len,&i_table);
    return inode_index;
}


//root directory functions
void init_root(){
    //creating empty inode which will be in position 0 since this is called before anything else. 
    sb.root_dir_inode = create_empty_inode();
}

void init_directory(){
    for(int i =0;i<MAX_FILE_NUM;i++){
        root_dir.list[i].inode_ptr=-1;
    }    
}



/*returns the index of a file if it exists otherwise returns -1*/
int file_exists(char * filename){
    for(int i = 0; i< MAX_FILE_NUM;i++){
    	char *current_filename = root_dir.list[i].filename;
	if(strcmp(current_filename,filename)==0){
            return i;
	} 
    }
    return -1; 
}


//NEED TO IMPLEMENT HARD DISK SAVING OF ROOT DIRECTORY
int new_file_dir(char *filename,int inode_num){
    int root_num = root_dir.first_free;
    root_dir.list[root_num].inode_ptr = inode_num;

    for(int i = root_dir.first_free;i<MAX_FILE_NUM;i++){
	if(root_dir.list[i].inode_ptr<0){
	    root_dir.first_free = i;
	    break;
	}
    }
    if(root_num == root_dir.first_free){
	printf("Root Directory is full\n");
	root_dir.list[root_num].inode_ptr = inode_num;
        return -1;
    }
    strncpy(root_dir.list[root_num].filename,filename,strlen(filename));
    return root_num;
}

int delete_file_dir(int dir_pos){
    if(dir_pos<0||dir_pos>MAX_FILE_NUM-1) return -1;
    root_dir.list[dir_pos].inode_ptr =-1;
    strcpy(root_dir.list[dir_pos].filename,"");
    return 0; 
}



//fdt directory functions
void init_fdt(){
   f_table.next_free = 0;
   for(int i = 0; i< MAX_FILE_NUM;i++){
       f_table.fdt[i].inode = -1;
       f_table.fdt[i].rwptr = 0;
   } 
}

int new_fd(int inodeNum){
    int new_fdt = f_table.next_free;
    f_table.fdt[new_fdt].inode = inodeNum;

    for(int i = f_table.next_free;i<MAX_FILE_NUM;i++){
	if(f_table.fdt[i].inode < 0){
            f_table.next_free = i;
	    break;
        }
    }

    if(f_table.next_free == new_fdt){
        printf("The file descriptor table is full, can no longer add.\n");
        f_table.fdt[new_fdt].inode= -1;
        return -1;
    }
    f_table.fdt[new_fdt].inode = inodeNum; 
    f_table.fdt[new_fdt].rwptr = 0;
    return new_fdt; 
}

int remove_fd(int file_ptr){
    if(file_ptr<0 || file_ptr>MAX_FILE_NUM - 1){
	printf("Invalid index for deleting a file descriptor\n");
        return -1;
    }
    f_table.fdt[file_ptr].inode = 0;
    f_table.fdt[file_ptr].rwptr = 0; 
    if(file_ptr < f_table.next_free){
        f_table.next_free = file_ptr;
    }
    return 0; 
}

int verify_in_fd(int inode){
for(int i =0;i<MAX_FILE_NUM;i++){
    if(f_table.fdt[i].inode==inode){
	return i;
    }
  }
  return -1;
}


//create the file system
void mksfs(int fresh){
    if(fresh){
	printf("Making new file system\n");
	//initializing the superblock data
	init_superblock();
	init_fresh_disk(OLIS_DISK,BLOCK_SZ,NUM_BLOCKS);
	init_inode_table();
	init_bitmap();
	//writing the superblock and setting to occupied in the bitmap
	write_blocks(0,1,&sb);
	bm.index[0] = BLOCK_OCCUPIED;
	//writing the inode table and setting to occupied in the bitmap
	write_blocks(1,sb.inode_table_len,&i_table);
	for(int i =1;i<sb.inode_table_len+1;i++){
	   bm.index[i]=BLOCK_OCCUPIED;
	}
	//writing bitmap and setting last blocks to the bitmap blocks
	for(int i =BITMAP_START;i<BITMAP_START+NUM_BITMAP_BLOCKS;i++){
            bm.index[i] = BLOCK_OCCUPIED;
	}
	write_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
	
	init_root();
	init_fdt();
        init_directory();
	}
	else{
	    printf("reopening file system\n");
	    read_blocks(0,1,&sb);
	    //open inode table
            read_blocks(1,sb.inode_table_len,&i_table);
	    read_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
	    init_fdt();
	}
    return;
}

int sfs_getnextfilename(char *fname) {
    //Implement sfs_getnextfilename here
    return 0;
}
int sfs_getfilesize(const char* path) {
    //Implement sfs_getfilesize here
    return 0;
}

//sfs_fopen helper functions
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


int sfs_fopen(char *name) {  
    //validating filename
    if(!validate_filename(name)){
    	return -1;
    }
    
    //check if file already exists
    int file_number = file_exists(name);   
    if(file_number<0){
        
        int inode_num = create_empty_inode();
	
        if(inode_num<0) return -1;
        
        int fdt_num = new_fd(inode_num);
 
	if(fdt_num<0) return -2;
	f_table.fdt[fdt_num].inode = inode_num;
	f_table.fdt[fdt_num].rwptr = 0;

        int file_dir_num = new_file_dir(name,inode_num);
        if(file_dir_num<0) return -3;

        return fdt_num;		
    }
    else{
	int inode_ptr = root_dir.list[file_number].inode_ptr;
        int infd = verify_in_fd(inode_ptr);
        if(infd<0){
            return -4;
	}
        int file_desc = new_fd(inode_ptr);
	if(file_desc<0){
	    return -5;
	}
	return file_desc; 	
    }
}


int sfs_fclose(int fileID){

	//Implement sfs_fclose here
    //fdt[0].inode = 0;
    //fdt[0].rwptr = 0;
    return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length){
	//file_descriptor* f = &fdt[fileID];
	//inode_t* n = &table[fileID];
	/*
	 *We know block 1 is free because this is a canned example.
	 *You should call a helper function to find a free block.
	 */
	//int block = 20;
	//n -> data_ptrs[0] = block;
	//n -> size += length;
	//f -> rwptr += length;
	//write_blocks(block,1,(void *) buf);
	return 0;
}

int sfs_fread(int fileID, char *buf, int length){
			//Implement sfs_fread here
	    //file_descriptor* f = &fdt[fileID];
	    //inode_t* n = i_table.[f->inode];

	    //int block = n->data_ptrs[0];
	    //read_blocks(block, 1, (void*) buf);
	    return 0;
}


int sfs_fseek(int fileID, int loc){
	//MINIMAL IMPLEMENTATION, ADD SOME TYPE OF ERROR CHECKING
	//fdt[fileID].rwptr = loc;
	return 0;
}

int sfs_remove(char *file){
	//implement sfs_remove here
	return 0;
}
