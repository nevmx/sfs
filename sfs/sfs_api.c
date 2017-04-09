#include "sfs_api.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define DISK_NAME "ecse427disk"
#define N_DATA_BLOCKS 1024
#define B_SIZE 1024
#define N_INODES 200

typedef struct _inode_t {
    int32_t size;
    int32_t direct[14];
    int32_t indirect;
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
direntry_t *rootdir;

int rootdir_stale = 1;
int inodes_stale = 1;

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
        root.direct[13] = -1;

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
        in[0].direct[0] = 13;
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

        // Create new free bitmap - all blocks are free except first 13 (inodes) and 14th (root dir)
        for (int i = 0; i < N_DATA_BLOCKS; i++) {
            if (i < 14) {
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

    inodes_stale = 0;

    // Free allocated memory that is no longer needed
    free(superblock_raw);
    free(inodes_raw);
    free(fbm_raw);

    // print_superblock(superblock);
    // printf("i-node 0 size: %i\n", inodes[0].size);
    // printf("i-node 0 direct 1: %i\n", inodes[0].direct[0]);
    // printf("i-node 0 direct 2: %i\n", inodes[0].direct[1]);
    // printf("i-node 1 size: %i\n", inodes[1].size);
    // printBytes(fbm, 20);

    read_root_dir();
}
int ssfs_fopen(char *name){
    print_dir();

    // Does the file already exist?
    int file_exists = find_file(name);

    if (file_exists != -1) {
        printf("File %s exists already, will be opened.\n", name);
    } else {
        printf("File %s does not exist, will be created.\n", name);
    }

    // File does not exist, create it
    if (file_exists == -1) {
        // 1. Find an empty i-node
        inode_t *free_inode = NULL;

        int i;
        for (i = 0; i < N_INODES; i++) {
            if (inodes[i].size == -1) {
                free_inode = &inodes[i];
                break;
            }
        }

        if (free_inode == NULL) {
            printf("No more empty inodes!\n");
            return -1;
        } else {
            printf("Using inode #%i\n", i);
        }

        // 2. Set inode size to 0
        free_inode->size = 0;

        // 3. Add to root directory
        int32_t dirsize = inodes[0].size / 16;
        rootdir = (void*)realloc(rootdir, (dirsize + 1) * sizeof(direntry_t));
        strncpy(rootdir[dirsize].filename, name, 15);
        rootdir[dirsize].inode = i;
        file_exists = i;

        // 4. Increment root dir size
        inodes[0].size += sizeof(direntry_t);
    }

    print_dir();
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
    // Todo: if file is open, close it.

    // First, find the file.
    int file_inode_number = find_file(file);

    if (file_inode_number == -1) {
        printf("File %s does not exist. Cannot remove.\n", file);
    } else {
        printf("File has inode #%s, deleting.\n", file_inode_number);
    }

    return 0;
}

int find_file(char* name) {
    // Update cache if needed
    if (rootdir_stale != 0) {
        read_root_dir();
    }

    // Update inode cache if needed
    if (inodes_stale != 0) {
        read_inodes();
    }

    int file_exists = -1;
    int32_t dirsize = inodes[0].size / 16;
    for (int i = 0; i < dirsize; i++) {
        if (strcmp(rootdir[i].filename, name) == 0) {
            file_exists = rootdir[i].inode;
        }
    }
    return file_exists;
}

void read_inodes() {
    // Raw data for inodes
    unsigned char *inodes_raw = (void*)calloc(B_SIZE * 13, sizeof(unsigned char));

    // Read inodes
    if (read_blocks(1, 13, inodes_raw) != 13) {
        printf("%s", "I-node read error.\n");
        return;
    }

    // Copy to cache
    memcpy(inodes, inodes_raw, sizeof(inode_t) * N_INODES);
    free(inodes_raw);

    inodes_stale = 0;
}

void read_root_dir() {
    if (inodes_stale == 1) {
        read_inodes();
    }

    // Number of files in root directory
    int32_t dirsize = inodes[0].size / 16;
    if (dirsize < 1) {
        rootdir_stale = 0;
        return;
    }

    // Number of blocks to read - 64 entries per block
    int blocks_to_read = dirsize / 64;
    if (dirsize % 64 != 0) {
        blocks_to_read++;
    }

    printf("Blocks to read: %i\n", blocks_to_read);

    // Allocate memory for root dir
    free(rootdir);
    rootdir = calloc(dirsize, sizeof(direntry_t));

    // Variable to hold read blocks from disk
    unsigned char read_blocks_raw[blocks_to_read][B_SIZE];

    int read_blocks_count = 0;
    int32_t current_data_block = inodes[0].direct[0];

    while (read_blocks_count < blocks_to_read) {
        // Read a block
        printf("Reading block %i\n", current_data_block);
        read_blocks(current_data_block, 1, read_blocks_raw[read_blocks_count++]);
        current_data_block = inodes[0].direct[read_blocks_count];
    }

    int read_entries = 0;
    // Interpret data as directory entries
    for (int i = 0; i < blocks_to_read; i++) {
        for (int j = 0; j < 64 && read_entries < dirsize; j++) {
            memcpy(rootdir + read_entries, read_blocks_raw[i] + (j * sizeof(direntry_t)), sizeof(direntry_t));
            read_entries++;
        }
    }

    rootdir_stale = 0;
}

void print_dir() {
    // Update cache if needed
    if (rootdir_stale != 0) {
        read_root_dir();
    }

    // Number of files in root directory
    int32_t dirsize = inodes[0].size / 16;
    if (dirsize < 1) {
        printf("Empty directory.\n");
        return;
    }

    printf("Directory Listing:\n");

    for (int i = 0; i < dirsize; i++) {
        printf(" - %s, %i\n", rootdir[i].filename, rootdir[i].inode);
    }
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
    printf("J-node: Size: %i\n", sb->root.size);
    for (int i = 0; i < 14; i++) {
        printf("        Direct: %i\n", sb->root.direct[i]);
    }
}
