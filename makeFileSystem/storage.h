//
// Created by mehmet on 23.05.2020.
//

#ifndef OS_MIDTERM_PART1_STORAGE_H
#define OS_MIDTERM_PART1_STORAGE_H


extern char *storage;

int storage_init();
int read_block(int block_no, char *buffer, int size);
int write_block(int block_no, char *buffer, int size);
int mount_storage(char *file_path);
int umount_storage(char *file_path);

#endif //OS_MIDTERM_PART1_STORAGE_H