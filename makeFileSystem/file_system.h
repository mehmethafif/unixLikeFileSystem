//
// Created by mehmet on 23.05.2020.
//

#ifndef OS_MIDTERM_PART1_FILE_SYSTEM_H
#define OS_MIDTERM_PART1_FILE_SYSTEM_H



#include <sys/time.h>
#include "storage.h"
#include "main.h"

#define MAX_FILENAME 20
#define MAX_DIR_ENTRY (BLOCK_SIZE-4) / sizeof(DirEntry)
#define MAX_TABLE_ENTRY BLOCK_SIZE / sizeof(int)

typedef enum {file, dir} file_t;

typedef struct{
    int root_dir_inode;
    int free_block_count;
    int free_inode_count;
    int block_size;
    int inode_count;
} SuperBlock;

typedef struct{
    file_t type;
    struct timeval last_modified;
    int size;
    int block_count;
    int direct_block[10];
    int indirect_block;
    int double_indirect_block;
} Inode;

typedef struct{
    int inode;
    char name[MAX_FILENAME];
} DirEntry;

typedef struct{
    int num_entry;
    DirEntry *dir_entry;
} Directory;

extern char *inode_map;
extern char *block_map;
extern SuperBlock superBlock;

int init_fs();
Directory* create_dir_block();
int fs_load();
int fs_release();
int get_block_size_n_inode_count(int *BLOCK_SIZE_ARG, int *INODE_COUNT_ARG);
int get_inode_no(char *name);
int get_dir_inode_no(char *name);
int make_dir(char *name);
int remove_dir(char *name);
int write_dir_block(int block_no, Directory *buffer);
int read_dir_block(int block_no, Directory *buffer);
int list_dir(char *name);
int find_block_no(Inode *inode, int block_count);
int write_file(char *new_name, char *from_name);
int assign_data_blocks(int inode_no);
int read_file(char *file_name, char *to_name);
int clean_data_blocks(int inode_no);
int delete_file(char *name);
int simple_dumpe2fs();


#endif //OS_MIDTERM_PART1_FILE_SYSTEM_H