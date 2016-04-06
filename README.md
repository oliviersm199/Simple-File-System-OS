# comp310as2
Creating a UNIX-like simple file system in Linux using FUSE wrappers.

Simplifications Include:

- Limited length filenames (Upper Limit of 16)
- Limited length file extensions (Upper limit of 3)
- No subdirectories (only a single root directory)
- Number of files is limited to number of blocks
- File System is implemented over a disk emulator







Data Structures in Memory Overview

typedef struct {
    int magic;
    int block_size;
    int fs_size;
    int inode_table_len;
    int root_dir_inode;
} superblock_t;

API INSTANCE: sb

typedef struct {
    char index[NUM_BLOCKS];
    int first_free_block;
} bitmap;

API INSTANCE: bm

typedef struct {
    char inuse;
    unsigned int size;
    unsigned int data_ptrs[NUM_DATAPTRS];
    unsigned int indirect_data_block;
} inode_t;

typedef struct{
  inode_t index[NUM_INODES];
  int first_free_inode;
} inode_table;

API_INSTANCE: i_table


typedef struct {
    int inode;
    int rwptr;
} file_descriptor;

typedef struct {
    file_descriptor fdt[MAX_FILE_NUM];
    int next_free;
} fdt_table;

API INSTANCE: f_table

typedef struct {
    char filename[20];
    int inode_ptr;
} file_entry;

typedef struct {
    file_entry list[MAX_FILE_NUM];
    int first_free;
    int first_file;
    int last_file;
} root_directory;

API INSTANCE: root_dir
