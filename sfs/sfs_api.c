#include "sfs_api.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define DISK_NAME "ecse427disk"
#define N_DATA_BLOCKS 1024
#define B_SIZE 1024
#define N_INODES 200

typedef struct _inode_t {
    uint32_t size;
    uint32_t direct[14];
    uint32_t indirect;
} inode_t;

typedef struct _superblock_t {
    unsigned char magic[4];
    uint32_t bsize;
    uint32_t n_blocks;
    uint32_t n_inodes;
    inode_t root;
} superblock_t;

// typedef struct _direntry_t {
//     unsigned char filename[10];
//     uint8_t inode;
// }

superblock_t *superblock;
uint8_t *fbm;

void mkssfs(int fresh){
    // Initialize some vars used for caching
    superblock = (void*)malloc(sizeof(superblock_t));
    fbm = (void*)calloc(N_DATA_BLOCKS, sizeof(uint8_t));

    // Variables to hold raw byte arrays for superblock and fbm
    unsigned char *superblock_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));
    unsigned char *fbm_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));

    // If flag is not set, open an existing disk
    if (fresh == 0) {
        if (init_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2) != 0) {
            printf("%s\n", "Disk init error.");
            return;
        }

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
    } else {
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

        // Byte array containing superblock - copy superblock into it
        // Pad the remaining stuff with zeroes
        memcpy(superblock_raw, &sb, sizeof(superblock_t));
        for (int i = sizeof(superblock_t); i < 1024; i++) {
            superblock_raw[i] = 0;
        }

        // Create new free bitmap - all blocks are free
        for (int i = 0; i < N_DATA_BLOCKS; i++) {
            fbm_raw[i] = 1;
        }

        fbm_raw[1023] = 3;

        // Initialize disk and write superblock and FBM
        if (init_fresh_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2) != 0) {
            printf("%s", "Disk init error.\n");
            return;
        }
        if (write_blocks(0, 1, superblock_raw) != 1) {
            printf("%s", "Superblock write error.\n");
            return;
        }
        if (write_blocks(N_DATA_BLOCKS + 1, 1, fbm_raw) != 1) {
            printf("%s", "FBM write error.\n");
            return;
        }
    }

    // Copy all info to the cache
    memcpy(superblock, superblock_raw, sizeof(superblock_t));
    memcpy(fbm, fbm_raw, sizeof(uint8_t) * 1024);

    // Free allocated memory that is no longer needed
    free(superblock_raw);
    free(fbm_raw);
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

void print_superblock(superblock_t *sb) {
    printf("Magic: %02x%02x%02x%02x\n", sb->magic[0], sb->magic[1], sb->magic[2], sb->magic[3]);
    printf("BSize: %i\n", sb->bsize);
    printf("N of Blocks: %i\n", sb->n_blocks);
    printf("N of i-nodes: %i\n", sb->n_inodes);
}
