#include "sfs_api.h"
#include "stdio.h"
#include "stdlib.h"

#define DISK_NAME "ecse427disk"
#define N_DATA_BLOCKS 1024
#define B_SIZE 1024
#define N_INODES 200

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
    // If flag is not set, open an existing disk
    if (fresh == 0) {
        if (init_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2) != 0) {
            return;
        }

        // Data structures to hold raw data from file
        unsigned char *superblock_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));
        unsigned char *fbm_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));

        // Read superblock
        if (read_blocks(0, 1, superblock_raw) != 1) {
            printf("%s", "Superblock read error.\n");
            return;
        }

        // Read FBM
        if (read_blocks(N_DATA_BLOCKS + 1, 1, fbm_raw) != 1) {
            printf("%s", "FBM read error.\n");
            return;
        }
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
    sb.root = root;

    // Create new free bitmap - all blocks are free
    unsigned char fbm[N_DATA_BLOCKS];
    for (int i = 0; i < N_DATA_BLOCKS; i++) {
        fbm[i] = 1;
    }

    // Initialize disk and write superblock and FBM
    if (init_fresh_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2) != 0) {
        printf("%s", "Disk init error.\n");
        return;
    }
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

void printBytes(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%x\n", data[i]);
    }
}
