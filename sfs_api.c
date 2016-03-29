#include "disk_emu.h"

int seen = 0;

#define OLIS_DISK "sfs_disk.disk"
#define BLOCK_SZ 1024
#define NUM_BLOCKS 100
#define NUM_INODES 10


#define NUM_INODE_BLOCKS (sizeof(inode_t) * NUM_INODES / BLOCK_SZ + 1)

superblock_t sb;
inode_t table[NUM_INODES];
file_descriptor fdt[NUM_INODES];

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
		init_superblock();
		init_fresh_disk(OLIS_DISK,BLOCK_SZ,NUM_BLOCKS);
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

int sfs_getnextfilename(char *fname); // get the name of the next file in directory

int sfs_getfilesize(const char* path); // get the size of the given file

int sfs_fopen(char *name); // opens the given file

void sfs_fclose(int fileID); // closes the given file

void sfs_fwrite(int fileID, char *buf, int length){
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

void sfs_fread(int fileID, char *buf, int length); // read characters from disk into buf


void sfs_fseek(int fileID, int loc){

	//MINIMAL IMPLEMENTATION, ADD SOME TYPE OF ERROR CHECKING
	fdt[fileID].rwptr = loc;
	return 0;
}

int sfs_remove(char *file){
	//implement sfs_remove here
	return 0;
}
