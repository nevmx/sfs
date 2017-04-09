#include "stdio.h"

void main() {
    printf("%s", "- Test 3 -\n");

    mkssfs(0);
    ssfs_fopen("sfs.exe");
    ssfs_fopen("main.c");
    ssfs_fopen("test.txt");
    ssfs_remove("sfs.exe");
    ssfs_remove("main.c");
    ssfs_remove("test.txt");
    ssfs_remove("..");
}
