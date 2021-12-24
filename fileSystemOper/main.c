#include <stdio.h>
#include <stdlib.h>

#include "storage.h"
#include "file_system.h"



int BLOCK_SIZE;
int BLOCK_COUNT;
int INODE_COUNT;

int main(int argc, char *argv[]) {

    storage_init();

    mount_storage(argv[1]);

    get_block_size_n_inode_count(&BLOCK_SIZE, &INODE_COUNT);
    BLOCK_COUNT = 1024*1024 / BLOCK_SIZE;

    init_fs();

    fs_load();

    // DO OPERATION
    int op = get_command_no(argv[2]);


    if (op == -1){
        printf("Invalid operation entered!\n");
    } else if (op == 0){ //list
        list_dir(argv[3]);
    } else if (op == 1){ //mkdir
        make_dir(argv[3]);
    } else if (op == 2){ //rmdir
        remove_dir(argv[3]);
    } else if (op == 3){ //dumpe2fs
        simple_dumpe2fs();
    } else if (op == 4){ //write
        write_file(argv[3], argv[4]);
    } else if (op == 5){ //read
        read_file(argv[3], argv[4]);
    } else if (op == 6){ //del
        delete_file(argv[3]);
    }


    fs_release();

    umount_storage(argv[1]);

    free(storage);

    return 0;
}
