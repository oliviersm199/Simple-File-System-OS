#include "disk_emu.h"
#include "sfs_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//instances of in memory data structures
superblock_t sb;
inode_table i_table;
bitmap bm;
root_directory root_dir;
fdt_table f_table;


//intializiation of in memory data structure sizes
int dir_size = sizeof(root_dir.list)/sizeof(root_dir.list[0]);
int fdt_size = sizeof(f_table.fdt)/sizeof(f_table.fdt[0]);
int i_table_size = sizeof(i_table.index)/sizeof(i_table.index[0]);
int bm_size = sizeof(bm.index)/sizeof(bm.index[0]);



//initialization of superblock 
void init_superblock(){
    sb.magic = 0xACBD0005;
    sb.block_size = BLOCK_SZ;
    sb.fs_size = NUM_BLOCKS * BLOCK_SZ;
    sb.inode_table_len = NUM_INODE_BLOCKS;
    sb.root_dir_inode = 0;
}

//saving functionalitie
void save_superblock_table(int sb_block){
    write_blocks(sb_block,1,&sb);
}

void save_inode_table(){
    write_blocks(1,sb.inode_table_len,&i_table);
}


void save_bitmap_table(){
    write_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
}

void save_root(){
    inode_t * root_inode = &i_table.index[sb.root_dir_inode];
    int num_data_blocks = (sizeof(root_dir)/BLOCK_SZ)+ 1;
    write_blocks(root_inode -> data_ptrs[0],num_data_blocks,&root_dir);
}

//intialization functionalities for new file system (except superblock initialization)
void init_bitmap(){
    int i;
    for(i=0;i<bm_size;i++){
        bm.index[i] = BLOCK_AVALIABLE;
    }
    bm.first_free_block = 0;
    save_bitmap_table();
}

void init_inode_table(){
    for(int i = 0 ; i<i_table_size;i++){
        i_table.index[i].inuse = INODE_FREE;
        i_table.index[i].ptrs_used = 0;
    }
    i_table.first_free_inode = 0;
    save_inode_table();
}

void init_fdt(){
   f_table.next_free = 0;
   for(int i = 0; i<fdt_size;i++){
       f_table.fdt[i].inode = -1;
       f_table.fdt[i].wptr = 0;
       f_table.fdt[i].rptr = 0;
   } 
}

//block functionalities
int occupy_block(){
    int return_value =  bm.first_free_block;    
    //updating the last_free_block parameter
    for(int i =bm.first_free_block+1;i<bm_size;i++){
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
    save_bitmap_table();
    return return_value; 
}

int release_block(int release){
    //block is free to overwrite.
    if(release>=0 && release<bm_size){
        if(bm.index[release]==BLOCK_OCCUPIED){
            bm.index[release] = BLOCK_AVALIABLE;
	    if(bm.first_free_block>release){
		bm.first_free_block=release;
	    }
	    save_bitmap_table();
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

int get_block_assignments(inode_t * i_ptr,int blocks_needed){
    //finding out the start of new blocks
    int ptrs_used = i_ptr -> ptrs_used;
    int max_index = ptrs_used + blocks_needed;
    int indirect_used = (ptrs_used-DIRECT_PTRS<=0) ? 0: ptrs_used - DIRECT_PTRS;
    //if we need more pointers than we can support, return -1.
    if(max_index > MAX_DATA_PTRS){
	return -1;	
    }
    //try to assign the number of new blocks that are needed. 
    //if not possible, release all the blocks that we allocated and return -2.
    int blocks_assigned[blocks_needed]; 
    for(int i =0; i< blocks_needed;i++){
	blocks_assigned[i] = occupy_block();
	if(blocks_assigned[i]<0){
	    for(int j=0;j<=i;j++){
		release_block(blocks_assigned[i]);
	    }
	    return -2;
	}
    }

	
    //j is the index for the dataptrs we acquired.
    int j = 0; 
    for(int i = ptrs_used;i<max_index;i++){
	if(i<MAX_DATA_PTRS-1){
	    i_ptr -> data_ptrs[i]=blocks_assigned[j];
	    j++;
        }
        else{
	    if(i_ptr -> indirect_ptr<0){
		i_ptr -> indirect_ptr = occupy_block();
		if(i_ptr -> indirect_ptr < 0){
		    for(int q = 0;q<blocks_needed;q++){
			release_block(blocks_assigned[q]);
		    }
		    return -3;
		}	
	    }
	    int temp_inode_array[MAX_DATA_PTRS-DIRECT_PTRS];
	    read_blocks(i_ptr->indirect_ptr,1,temp_inode_array);
	    while(j<blocks_needed){
		temp_inode_array[indirect_used+j] = blocks_assigned[j];
		j++;
	    }
            write_blocks(i_ptr -> indirect_ptr,1,temp_inode_array);
	    break;
        }
    }
    i_ptr -> ptrs_used += blocks_needed;
    save_inode_table();
    return 0;      
}


//inode functionalities
int occupy_inode(){
    int return_value = i_table.first_free_inode;
    for(int i =return_value+1;i<i_table_size;i++){
	if(i_table.index[i].inuse == INODE_FREE){
		i_table.first_free_inode = i;
		break;
	}
    }
    if(return_value == i_table.first_free_inode){
        return -1;
    }
    i_table.index[return_value].inuse =INODE_IN_USE;
    return return_value;
}

int create_empty_inode(){
    int inode_index = occupy_inode();
    if(inode_index<0){
	printf("No free inode\n");
	return -1;
    }
    inode_t * newInode = &i_table.index[inode_index];
    newInode -> size = 0;
    save_inode_table();
    return inode_index;
}

int release_inode(int release){
    if(release>=0 && release < i_table_size){
    if(i_table.index[release].inuse==INODE_IN_USE){
      	i_table.index[release].inuse = INODE_FREE;
        if(i_table.first_free_inode> release) i_table.first_free_inode = release;
	    i_table.index[release].size = 0;
	    i_table.index[release].ptrs_used=0;
	    save_inode_table();
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

int release_inode_blocks(int inodeNum){
    //first validating that it is a valid inodeNum
    if(inodeNum<0 || inodeNum > i_table_size-1){
	return -1;
    }	

    //checking if it is in use otherwise why would i free?
    if(i_table.index[inodeNum].inuse == INODE_FREE){
	return -2;
    }
    inode_t *  i_ptr = &i_table.index[inodeNum];
    
    int ptrs_used = i_ptr -> ptrs_used;
    for(int i =0;i<DIRECT_PTRS && ptrs_used>0;i++){
	release_block(i_ptr -> data_ptrs[i]);
	ptrs_used--;	
    }
   
    if(ptrs_used < 0){
	return 0;
    }
    
    //we have to load the dataptrs from the indirect block and release them one at a time and then free this block
    if(i_ptr -> indirect_ptr < 0){
	return -3;
    }
    int temp_inode_array[MAX_DATA_PTRS-DIRECT_PTRS];
    read_blocks(i_ptr->indirect_ptr,1,temp_inode_array);
    int i = 0;
    while(ptrs_used>0&&i<MAX_DATA_PTRS-DIRECT_PTRS){
	if(temp_inode_array[i]>0){
	   release_block(temp_inode_array[i]); 
        }
	ptrs_used--;
        i++;
    }
    release_block(i_ptr -> indirect_ptr);
    i_ptr -> indirect_ptr = 0;
    save_inode_table();
    return 1;
}

//root directory functions
void init_root(){
    //creating empty inode which will be in position 0 since this is called before anything else. 
    sb.root_dir_inode = create_empty_inode();
    inode_t * root_inode = &i_table.index[sb.root_dir_inode];
    
    int num_data_blocks = (sizeof(root_dir)/BLOCK_SZ)+ 1;
    
    for(int i =0;i<num_data_blocks;i++){
        root_inode -> data_ptrs[i] = occupy_block();
    }

    //initializing the inode values to -1 (for unoccupied inodes)
    for(int i = 0; i<dir_size;i++){
	root_dir.list[i].inode_ptr = -1;
    }
    save_inode_table();
    save_root();
}




/*returns the index of a file if it exists otherwise returns -1*/
int file_exists(char * filename){
    for(int i = 0; i< dir_size;i++){
    	char *current_filename = root_dir.list[i].filename;	
	if(strcmp(current_filename,filename)==0){
            return i;
	} 
    }
    return -1; 
}


int new_file_dir(char *filename,int inode_num){
    int root_num = root_dir.first_free;
    root_dir.list[root_num].inode_ptr = inode_num;

    for(int i = root_dir.first_free;i<dir_size;i++){
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
    save_root();
    return root_num;
}

int delete_file_dir(int dir_pos){
    if(dir_pos<0||dir_pos>dir_size-1) return -1;
    root_dir.list[dir_pos].inode_ptr =-1;
    strcpy(root_dir.list[dir_pos].filename,"");
    
    if(dir_pos<root_dir.first_free){
    	root_dir.first_free = dir_pos;
    }

    save_root();
    return 0; 
}


//fdt directory functions
int new_fd(int inodeNum){
    int new_fdt = f_table.next_free;
    f_table.fdt[new_fdt].inode = inodeNum;
    for(int i = f_table.next_free;i<fdt_size;i++){
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
    f_table.fdt[new_fdt].rptr = 0;
    f_table.fdt[new_fdt].wptr = 0;
    return new_fdt; 
}

int remove_fd(int file_ptr){
    if(file_ptr<0 || file_ptr>fdt_size - 1){
	printf("Invalid index for deleting a file descriptor\n");
        return -1;
    }
    if(f_table.fdt[file_ptr].inode<0){
	return -2;
    }
    f_table.fdt[file_ptr].inode = -1;
    f_table.fdt[file_ptr].wptr = 0;
    f_table.fdt[file_ptr].rptr=0;
 
    if(file_ptr < f_table.next_free){
        f_table.next_free = file_ptr;
    }
    return 0; 
}

int verify_in_fd(int inode){
for(int i =0;i<fdt_size;i++){
    if(f_table.fdt[i].inode==inode){
	return i;
    }
  }
  return -1;
}



//various helper functions
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


int print_inode_table(){
    printf("\n\n");
    for(int i=0;i<i_table_size;i++){
        if(i_table.index[i].inuse==INODE_IN_USE){
                    inode_t * i_ptr = &i_table.index[i];
                    printf("Inode Number:%d\n",i);
                    printf("\tSize:%d\n",i_ptr->size);
                    printf("\tPtrs Used:%d\n",i_ptr->ptrs_used);
                    for(int j=0;i_ptr->data_ptrs[j]>0;j++){
                        printf("\tData Ptr:%d\tData Block %d\n",j,i_ptr->data_ptrs[j]);
                    }
                }
            }
    printf("\n\n");
    return 1;
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
	int sb_block = occupy_block();
	save_superblock_table(sb_block);	
        //writing the inode table and setting to occupied in the bitmap
	for(int i =1;i<sb.inode_table_len+1;i++){
	    occupy_block();
        }
	save_inode_table();	
	//writing bitmap and setting last blocks to the bitmap blocks
	for(int i =BITMAP_START;i<BITMAP_START+NUM_BITMAP_BLOCKS;i++){
            bm.index[i] = BLOCK_OCCUPIED;      
        }
	init_root();
	init_fdt();
        save_bitmap_table();
        }
	else{
	    printf("reopening file system\n");
	    int disk_result = init_disk(OLIS_DISK,BLOCK_SZ,NUM_BLOCKS);
	    
	    if(disk_result<0) return;
	    
	    read_blocks(0,1,&sb);
	    
	    inode_t * i_ptr = &i_table.index[sb.root_dir_inode];
	    int start_block = i_ptr -> data_ptrs[0];
	    int num_blocks = sizeof(root_dir)/BLOCK_SZ + 1;
            
	    printf("Start block %d\n",start_block);
	    printf("Root Dir Inode%d\n",sb.root_dir_inode);
	    printf("Num blocks:%d\n",num_blocks);

	    read_blocks(start_block,num_blocks,&root_dir); 
	    
            read_blocks(1,sb.inode_table_len,&i_table);
	    read_blocks(BITMAP_START,NUM_BITMAP_BLOCKS,&bm);
	    
            init_fdt();
	}
    return;
}

int sfs_getnextfilename(char *fname) {
    int count = root_dir.next;
    for(int i=0;i<dir_size;i++){
	if( root_dir.list[i].inode_ptr>=0 && count==0 ){
            strcpy(fname,root_dir.list[i].filename);
	    root_dir.next++;
            return 1;
        }
        else if(root_dir.list[i].inode_ptr>=0){
	    count--;
	}
    } 
    root_dir.next = 0; 
    return 0;
}

int sfs_getfilesize(const char* path) {
    if(!validate_filename((char *)path)){
        return -1;
    }
    //check if file already exists
    int file_number = file_exists((char *)path);
    if(file_number<0){
	return -2;
    } 
    
    int inode_number = f_table.fdt[file_number].inode;
    return i_table.index[inode_number].size;
}

int sfs_fopen(char *name) {  
    
    if(!validate_filename(name)){
    	return -1;
    }
    
    int file_number = file_exists(name);   
    if(file_number<0){
        
        int inode_num = create_empty_inode();
	
        if(inode_num<0) return -1;
        
        int fdt_num = new_fd(inode_num);
 
	if(fdt_num<0) return -2;
	f_table.fdt[fdt_num].inode = inode_num;
	f_table.fdt[fdt_num].rptr = 0;
	f_table.fdt[fdt_num].wptr = 0;
        int file_dir_num = new_file_dir(name,inode_num);
        if(file_dir_num<0) return -3;

        return fdt_num;		
    }
    else{
	int inode_ptr = root_dir.list[file_number].inode_ptr;
        int infd = verify_in_fd(inode_ptr);
        if(infd>=0){
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
    return remove_fd(fileID);
}
int sfs_fwrite(int fileID, const char *buf, int length){
	if(fileID<0||fileID >=fdt_size){
	    return -1;
        }
	
	int inode = f_table.fdt[fileID].inode;
	if(inode<0){
	  return -2;
        }
	inode_t * n = &i_table.index[inode];
        file_descriptor * f = &f_table.fdt[fileID];
	int blocks_used = n-> ptrs_used; 
        int wptr_start_block = f -> wptr/BLOCK_SZ;
	int blocks_writing = length/BLOCK_SZ+1;
	int blocks_needed = (blocks_writing-blocks_used)+wptr_start_block;
 	
        if(get_block_assignments(n,blocks_needed)<0){
	    return -3; 
        }
	int buf_written = 0;
        int to_write = length;
	char temp_buffer[BLOCK_SZ];
        for(int i =wptr_start_block;i<blocks_writing + wptr_start_block;i++){
	    int write_start = (f -> wptr%BLOCK_SZ);
            int block = n -> data_ptrs[i];
	    read_blocks(block,1,(void*)temp_buffer);
            int write_amount = (to_write<BLOCK_SZ-write_start) ? to_write:BLOCK_SZ-write_start;
	    char * temp_buf = (temp_buffer + write_start);
	    const char * buf_res = (buf + buf_written);
            strncpy(temp_buf,buf_res,write_amount); 
	    buf_written += write_amount;
            n -> size += (write_amount);
	    f -> wptr += (write_amount);
            to_write -= write_amount;
	    write_blocks(block,1,(void *)temp_buffer);
	}
	save_inode_table();
        return buf_written;
}

int sfs_fread(int fileID, char *buf, int length){
    if(fileID<0 || fileID >=fdt_size){
        return -1;
    }
    int inode = f_table.fdt[fileID].inode;
    if(inode<0){
	return -2;
    }
    file_descriptor * f = &f_table.fdt[fileID];
    inode_t * n = &i_table.index[inode];
    
    int rptr_start_block = f ->rptr/BLOCK_SZ;
    int rptr_blocks_read = length/BLOCK_SZ+1;
    int end_block = rptr_start_block +rptr_blocks_read; 
    int i =rptr_start_block;
    char temp_buffer[BLOCK_SZ];
    int read = 0;
    while(n->data_ptrs[i]>=0 && i<end_block && length > 0){
        int block = n->data_ptrs[i];
        read_blocks(block,1,(void*)temp_buffer);
        int to_read = ((length+ f->rptr)>BLOCK_SZ) ? BLOCK_SZ - (f->rptr%BLOCK_SZ):length;
        if(to_read > (n -> size)){
            to_read = n -> size;
	    length = 0;
        }
        int start_ptr = f->rptr % BLOCK_SZ;
        strncpy(buf+read,(temp_buffer+start_ptr),to_read);
        f->rptr += to_read;
	read += to_read;
	length -= to_read; 
        i++;
    }
    return read;
}


int sfs_fseek(int fileID, int loc){
	if(fileID<0||fileID>=fdt_size){
	    return -1;
        }
        int inode = f_table.fdt[fileID].inode;
	if(inode<0){
	    return -2;
        }
        if(loc<0 || loc > i_table.index[inode].size){ 
            f_table.fdt[fileID].wptr = loc;
            f_table.fdt[fileID].rptr = loc;
        }
        return 0;
}

int sfs_remove(char *file){
	//implement sfs_remove here
	if(!validate_filename(file)){
        	return -1;
	}

	int file_number = file_exists(file); 
	if(file_number<0){
	    return -2;
	}
	
	int inode_val = root_dir.list[file_number].inode_ptr;
	int delete_dir_r = delete_file_dir(file_number);
	if(delete_dir_r<0){
	    printf("Error deleting root directory\n");
	    return -3;
	}
	
	//take care of deleting any open file descriptors	
        int fd_pos = verify_in_fd(inode_val);
	if(fd_pos>0){
	    remove_fd(fd_pos);
	}

	//free the blocks in the inode
	release_inode_blocks(inode_val);
	
        //release the inode
	release_inode(inode_val);
	
        return file_number;
}
