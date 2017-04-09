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

typedef struct _direntry_t {
    unsigned char filename[15];
    uint8_t inode;
} direntry_t;

superblock_t *superblock;
uint8_t *fbm;
inode_t *inodes;

void mkssfs(int fresh){
    // Initialize some vars used for caching
    superblock = (void*)malloc(sizeof(superblock_t));
    fbm = (void*)calloc(N_DATA_BLOCKS, sizeof(uint8_t));
    inodes = (void*)calloc(N_INODES, sizeof(inode_t));

    // Variables to hold raw byte arrays for superblock and fbm
    unsigned char *superblock_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));
    unsigned char *fbm_raw = (void*)calloc(B_SIZE, sizeof(unsigned char));

    // Raw data for inodes
    unsigned char *inodes_raw = (void*)calloc(B_SIZE * 13, sizeof(unsigned char));

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

        // Read inodes
        if (read_blocks(1, 13, inodes_raw) != 13) {
            printf("%s", "I-node read error.\n");
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
        for (int i = 0; i < 13; i++) {
            root.direct[i] = i + 1;
        }

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

        // Initialize all inodes
        inode_t in[N_INODES];
        in[0].size = 0;
        for (int i = 1; i < N_INODES; i++) {
            in[i].size = -1;
        }
        memcpy(inodes_raw, &in, sizeof(inode_t) * N_INODES);

        // Byte array containing superblock - copy superblock into it
        // Pad the remaining stuff with zeroes
        memcpy(superblock_raw, &sb, sizeof(superblock_t));
        for (int i = sizeof(superblock_t); i < 1024; i++) {
            superblock_raw[i] = 0;
        }

        // Create new free bitmap - all blocks are free except first 13 (inodes)
        for (int i = 0; i < N_DATA_BLOCKS; i++) {
            if (i < 13) {
                fbm_raw[i] = 0;
            } else {
                fbm_raw[i] = 1;
            }
        }

        // Initialize disk and write superblock, inodes and FBM
        if (init_fresh_disk(DISK_NAME, B_SIZE, N_DATA_BLOCKS + 2) != 0) {
            printf("%s", "Disk init error.\n");
            return;
        }
        if (write_blocks(0, 1, superblock_raw) != 1) {
            printf("%s", "Superblock write error.\n");
            return;
        }
        if (write_blocks(1, 13, inodes_raw) != 13) {
            printf("%s", "I-node write error.\n");
            return;
        }
        if (write_blocks(N_DATA_BLOCKS + 1, 1, fbm_raw) != 1) {
            printf("%s", "FBM write error.\n");
            return;
        }
    }

    // Copy all info to the cache
    memcpy(superblock, superblock_raw, sizeof(superblock_t));
    memcpy(inodes, inodes_raw, sizeof(inode_t) * N_INODES);
    memcpy(fbm, fbm_raw, sizeof(uint8_t) * N_DATA_BLOCKS);

    // Free allocated memory that is no longer needed
    free(superblock_raw);
    free(inodes_raw);
    free(fbm_raw);

    printf("Size: %i\n", inodes[1].size);
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
