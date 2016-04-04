#ifndef _INCLUDE_SFS_API_H_
#define _INCLUDE_SFS_API_H_

#include <stdint.h>
//base definitions
#define MAXFILENAME 16
#define OLIS_DISK "sfs_disk.disk"
#define BLOCK_SZ 1024
#define NUM_BLOCKS 100
#define NUM_INODES 10
#define NUM_INODE_BLOCKS (sizeof(inode_t) * NUM_INODES / BLOCK_SZ + 1)


//bitmap definitions
#define NUM_BITMAP_BLOCKS (sizeof(bitmap)/BLOCK_SZ + 1)
#define BITMAP_START (NUM_BLOCKS - NUM_BITMAP_BLOCKS)
#define BLOCK_AVALIABLE 1
#define BLOCK_OCCUPIED 0  

//root directory definitions
#define MAX_FILE_NUM NUM_INODES

typedef struct {
    uint64_t magic;
    uint64_t block_size;
    uint64_t fs_size;
    uint64_t inode_table_len;
    uint64_t root_dir_inode;
} superblock_t;

typedef struct {
    unsigned int mode;
    unsigned int link_cnt;
    unsigned int uid;
    unsigned int gid;
    unsigned int size;
    unsigned int data_ptrs[12];
    // TODO indirect pointer
} inode_t;

/*
 * inode    which inode this entry describes
 * rwptr    where in the file to start
 */
typedef struct {
    uint64_t inode;
    uint64_t rwptr;
} file_descriptor;

typedef struct {
    char filename[16];
    char extension[3];
    file_descriptor fileptr;
} file_entry;

typedef struct {
    file_entry list[MAX_FILE_NUM];
} root_directory;

typedef struct {
    uint64_t index[NUM_BLOCKS];
    uint64_t rwptr; 
} bitmap;    



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
