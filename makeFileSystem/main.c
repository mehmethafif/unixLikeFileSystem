#include <stdlib.h>
#include "storage.h"
#include "file_system.h"


int BLOCK_SIZE;
int BLOCK_COUNT;
int INODE_COUNT;

int main(int argc, char *argv[]) {

    //setting Block size, block count and inode count from arguments
    BLOCK_SIZE = (atoi(argv[1]) * 1024);
    BLOCK_COUNT = (1024/atoi(argv[1]));
    INODE_COUNT = atoi(argv[2]);

    //initialize the storage
    storage_init();

    //initialize file system
    init_fs();

    //release filesystem to memory
    fs_release();

    //write storage memory to file
    umount_storage(argv[3]);

    //free storage memory
    free(storage);

    return 0;
}
