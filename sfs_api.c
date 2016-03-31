#include "disk_emu.h"
#include "sfs_api.h"

#include <stdio.h>

int seen = 0;

#define OLIS_DISK "sfs_disk.disk"
#define BLOCK_SZ 1024
#define NUM_BLOCKS 100
#define NUM_INODES 10


#define NUM_INODE_BLOCKS (sizeof(inode_t) * NUM_INODES / BLOCK_SZ + 1)

#define FREE_BLOCK 1
#define OCCUPIED_BLOCK 0

superblock_t sb;
inode_t table[NUM_INODES];
file_descriptor fdt[NUM_INODES];

//by default all values are considered free
int bitmap[NUM_BLOCKS] ={FREE_BLOCK}

void init_superblock(){
	sb.magic = 0xACBD0005;
	sb.block_size = BLOCK_SZ;
	sb.fs_size = NUM_BLOCKS * BLOCK_SZ;
	sb.inode_table_len = NUM_INODE_BLOCKS;
	sb.root_dir_inode = 0;
}

//create the file system
void mksfs(int fresh){
	if(fresh){
		printf("making new file system\n");

		//initializing the superblock data
		init_superblock();
		init_fresh_disk(OLIS_DISK,BLOCK_SZ,NUM_BLOCKS);
		//by default goes to block zero
		write_blocks(0,1,&sb);
		write_blocks(1,sb.inode_table_len,table);

	}

	else{
		printf("reopening file system\n");
		read_blocks(0,1,&sb);
		printf("Block Size is: %lu\n",sb.block_size);
		//open inode table
		read_blocks(1,sb.inode_table_len,table);
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
int sfs_fopen(char *name) {
		//Implement sfs_fopen here

    /*
     * For now, we only return 1
     */
    uint64_t test_file = 1;
    fdt[test_file].inode = 1;
    fdt[test_file].rwptr = 0;
		return test_file;
}

int sfs_fclose(int fileID){

	//Implement sfs_fclose here
    fdt[0].inode = 0;
    fdt[0].rwptr = 0;
		return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length){
	file_descriptor* f = &fdt[fileID];
	inode_t* n = &table[fileID];
	/*
	 *We know block 1 is free because this is a canned example.
	 *You should call a helper function to find a free block.
	 */
	int block = 20;
	n -> data_ptrs[0] = block;
	n -> size += length;
	f -> rwptr += length;
	write_blocks(block,1,(void *) buf);
	return 0;
}

int sfs_fread(int fileID, char *buf, int length){
			//Implement sfs_fread here
	    file_descriptor* f = &fdt[fileID];
	    inode_t* n = &table[f->inode];

	    int block = n->data_ptrs[0];
	    read_blocks(block, 1, (void*) buf);
		return 0;
}


int sfs_fseek(int fileID, int loc){
	//MINIMAL IMPLEMENTATION, ADD SOME TYPE OF ERROR CHECKING
	fdt[fileID].rwptr = loc;
	return 0;
}

int sfs_remove(char *file){
	//implement sfs_remove here
	return 0;
}


/*
Implementation of free space manager. The free space manager requires the following functionalities:
- Must keep an in memory bitmap (going to implement as an array) which represents which blocks are free.
- In memory bitmap must write to disk when a block is chosen to be written to.
- DECIDED ALLOCATION METHOD: Indexed Allocation and using a BitMap Scheme.
*/

//this method is designed to facilitate the use of writeblocks. It will ensure that the BitMap
//used to keep track of the free blocks is properly maintained.
int write_blocks_wrapper(int start_address, int nblocks, void *buffer){
	int safe = 0;
	for(int i =0;i<nblocks;i++){
		if(bitmap[start_address+i]!=1){
			safe = i;
		}
	}
	if(safe==-1){
		int result = write_blocks(start_address,nblocks,buffer);
		if(result>0){
			//no errors set the written blocks accordingly

		}
	}else{
		printf("Error, writing to occupied block Starting Address: %i Offset Block: %i.\n",start_address,safe);
		return -1;
	}
}

int set_free(int block_num){


}


int find_free_space(int space_required){
	int blocks_required = space_required/BLOCK_SZ;

}
