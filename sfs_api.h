#ifndef _INCLUDE_SFS_API_H_
#define _INCLUDE_SFS_API_H_

#include <stdint.h>
#include <stdlib.h>

//base definitions
#define MAXFILENAME 20
#define OLIS_DISK "sfs_disk.disk"
#define BLOCK_SZ 1024
#define NUM_BLOCKS 250
#define NUM_INODES 50
#define NUM_INODE_BLOCKS (sizeof(inode_table)/ BLOCK_SZ + 1)



//bitmap definitions

#define NUM_BITMAP_BLOCKS (sizeof(bitmap)/ BLOCK_SZ + 1)
#define BITMAP_START (NUM_BLOCKS - NUM_BITMAP_BLOCKS)
#define BLOCK_AVALIABLE '1'
#define BLOCK_OCCUPIED '0'  

//root directory definitions
//since we have a limited number of data blocks, we subsequently also have a maximum number of files.
//The minus 1 is to account for the superblock.
#define NUM_DATA_BLOCKS (NUM_BLOCKS - NUM_INODE_BLOCKS - NUM_BITMAP_BLOCKS-1)

//DEFINING THE MAX FILE SIZE
#define MAX_FILE_NUM (NUM_DATA_BLOCKS < NUM_INODES) ? NUM_DATA_BLOCKS:NUM_INODES

//inode definitions
#define INODE_IN_USE '1'
#define INODE_FREE '0'
#define NUM_DATAPTRS 12


typedef struct {
    int magic;
    int block_size;
    int fs_size;
    int inode_table_len;
    int root_dir_inode;
} superblock_t;


typedef struct {
    char index[NUM_BLOCKS];
    int first_free_block; 
} bitmap;    



typedef struct {
    char inuse;
    unsigned int size;
    unsigned int data_ptrs_used;
    unsigned int data_ptrs[NUM_DATAPTRS];
    unsigned int indirect_data_block; 
} inode_t;


typedef struct{ 
  inode_t index[NUM_INODES];
  int first_free_inode;
} inode_table;

typedef struct {
    int inode;
    int wptr;
    int rptr;
} file_descriptor;

typedef struct {
    file_descriptor fdt[MAX_FILE_NUM];
    int next_free;
} fdt_table;

typedef struct {
    char filename[20];
    int inode_ptr;
} file_entry;

typedef struct {
    file_entry list[MAX_FILE_NUM];
    int first_free;
    int next; 
} root_directory;


void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);

#endif //_INCLUDE_SFS_API_H_
