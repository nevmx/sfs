#include "sfs_api.h"

#DEFINE DISK_NAME ecse428disk
#DEFINE N_DATA_BLOCKS 1024;
#DEFINE B_SIZE 1014;
#DEFINE N_INODES 200;

typedef struct _inode_t {
    int size;
    int direct[14];
    int indirect;
} inode_t;

typedef struct _superblock_t {
    unsigned char magic[4];
    int bsize;
    int n_blocks;
    int n_inodes;
    inode_t root;
} superblock_t;

void mkssfs(int fresh){
    if (fresh == 0) {
        return;
    }

    // Define root j-node
    inode_t root;
    root.size = 0;

    // Define superblock for fresh disk
    superblock_t sb;
    sb.magic[0] = 0xAC;
    sb.magic[1] = 0xBD;
    sb.magic[2] = 0x00;
    sb.magic[3] = 0x05;
    sb.bsize = B_SIZE;
    sb.n_blocks = N_DATA_BLOCKS;
    sb.n_inodes = N_INODES;

    // Create new free bitmap - all blocks are free
    unsigned char fbm[N_DATA_BLOCKS];
    for (int i = 0; i < N_DATA_BLOCKS; i++) {
        fbm[i] = 1;
    }

    init_fresh_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2);
}
int ssfs_fopen(char *name){
    return 0;
}
int ssfs_fclose(int fileID){
    return 0;
}
int ssfs_frseek(int fileID, int loc){
    return 0;
}
int ssfs_fwseek(int fileID, int loc){
    return 0;
}
int ssfs_fwrite(int fileID, char *buf, int length){
    return 0;
}
int ssfs_fread(int fileID, char *buf, int length){
    return 0;
}
int ssfs_remove(char *file){
    return 0;
}

void main() {
    printf("%s", "Hello world!\n");
}
