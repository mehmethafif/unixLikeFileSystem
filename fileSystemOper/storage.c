//
// Created by mehmet on 23.05.2020.
//

#include "storage.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *storage;

int storage_init() {
    storage = (char *) malloc(1024*1024); //exacly 1MB
    memset(storage, 0, 1024*1024);
    if (!storage) {
        return -1;
    }
    return 0;
}

int read_block(int block_no, char *buffer, int size) {
    if (block_no >= BLOCK_COUNT) {
        return -1;
    }

    if(size == 0){
        memcpy(buffer, storage + (block_no * BLOCK_SIZE), BLOCK_SIZE);
    } else {
        memcpy(buffer, storage + (block_no * BLOCK_SIZE), size);
    }


    return 0;
}

int write_block(int block_no, char *buffer, int size) {
    if (block_no >= BLOCK_COUNT) {
        return -1;
    }
    if(size == 0){
        memcpy(storage + (block_no * BLOCK_SIZE), buffer, BLOCK_SIZE);
    } else {
        memcpy(storage + (block_no * BLOCK_SIZE), buffer, size);
    }



    return 0;
}

int mount_storage(char *file_path){
    FILE *fp = fopen(file_path, "r");
    int res;
    if(fp != NULL){
        fread(storage, 1, 1024*1024, fp);
        fclose(fp);
        return 0;
    }
    return -1;
}

int umount_storage(char *file_path){
    FILE * fp = fopen( file_path, "w" );
    int res;
    if(fp == NULL){
        fprintf( stderr, "umount_storage: file open error! %s\n", file_path );
        return -1;
    }
    fwrite( storage, 1, 1024*1024, fp );
    fclose(fp);
    return 0;
}

