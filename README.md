# comp310as2
Creating a UNIX-like simple file system in Linux using FUSE wrappers. Posix compliant. 

Simplifications Include:

- Limited length filenames (Upper Limit of 16)
- Limited length file extensions (Upper limit of 3)
- No subdirectories (only a single root directory)
- Number of files is limited to number of blocks
- File System is implemented over a disk emulator


<h2>API Overview</h2>

void mksfs(int fresh); //initializes the file system

int sfs_getnextfilename(char *fname); 

int sfs_getfilesize(const char* path);  

int sfs_fopen(char *name);

int sfs_fclose(int fileID);

int sfs_fwrite(int fileID, const char *buf, int length);

int sfs_fread(int fileID, char *buf, int length);

int sfs_fseek(int fileID, int loc);

int sfs_remove(char *file);
