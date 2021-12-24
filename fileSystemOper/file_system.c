//
// Created by mehmet on 23.05.2020.
//

#include "file_system.h"
#include "bitmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define DIV_UP(A, B) ((A % B) == 0 ? (A) / (B) : (A) / (B) + 1)

Inode *inode;
SuperBlock superBlock;
char *inode_map;
char *block_map;
Directory *root_dir;
int root_dir_block;
int root_directory_inode;
int inode_block_count;



int init_fs(){
    int i = 0;
    inode_block_count = DIV_UP(sizeof(Inode) * INODE_COUNT, BLOCK_SIZE);
    inode = (Inode *) malloc(BLOCK_SIZE*inode_block_count);

    inode_map = (char *) calloc(BLOCK_SIZE,1);
    block_map = (char *) calloc(BLOCK_SIZE,1);
    root_dir = create_dir_block();

    superBlock.free_block_count = BLOCK_COUNT - (3 + inode_block_count);
    superBlock.free_inode_count = INODE_COUNT;

    for( i = 0; i < INODE_COUNT / 8; i++){
        set_bit(inode_map, i, 0);
    }

    for( i = 0; i < BLOCK_COUNT / 8; i++ ){
        if( i < ( 3 + inode_block_count )){
            set_bit( block_map, i, 1 );
        } else{
            set_bit( block_map, i, 0 );
        }
    }

    return 0;
}


Directory* create_dir_block(){
    Directory *dir_block = malloc(sizeof(Directory));
    dir_block->num_entry = 0;
    dir_block->dir_entry = calloc(MAX_DIR_ENTRY, sizeof(DirEntry));
    return dir_block;
}

int create_table_block(){
    int *table = calloc(MAX_TABLE_ENTRY,sizeof(int));
    int new_block = get_free_block();
    write_block(new_block, (char*) table, 0);
    free(table);
    return new_block;
}

int fs_load(){
    int i;
    int index = 0;
    int inode_index = 0;

    read_block( 0, (char*) &superBlock, sizeof(SuperBlock));
    read_block( 1, inode_map, 0);
    read_block( 2, block_map, 0);


    for(i = 0; i < inode_block_count ; i++ ){
        index = i + 3;
        read_block(index, (char*) (inode + inode_index) ,0);
        inode_index += (BLOCK_SIZE / sizeof(Inode));
    }



    root_directory_inode = superBlock.root_dir_inode;
    root_dir_block = inode[root_directory_inode].direct_block[0];

    return 0;
}

int fs_release(){
    int i, index, inode_index = 0;

    write_block( 0, ( char* ) &superBlock, sizeof(SuperBlock));
    write_block( 1, inode_map, 0);
    write_block( 2, block_map, 0);


    for( i = 0; i < inode_block_count; i++ ){
        index = i + 3;
        write_block( index, ( char* ) ( inode + inode_index ), 0);
        inode_index += ( BLOCK_SIZE / sizeof( Inode ));
    }

    return 0;
}

int get_block_size_n_inode_count(int *BLOCK_SIZE_ARG, int *INODE_COUNT_ARG){
    SuperBlock temp_superblock;
    memcpy(&temp_superblock, storage, sizeof(SuperBlock));
    *BLOCK_SIZE_ARG = temp_superblock.block_size;
    *INODE_COUNT_ARG = temp_superblock.inode_count;
    return 0;
}

int get_inode_no(char *name){
    int cur_inode_no = superBlock.root_dir_inode;
    Directory *cur_dir;
    cur_dir = create_dir_block();
    int i, flag = 1;
    char* token = strtok(name, "/");
    while (token) {
        flag = 1;
        //token here get inode no
        read_dir_block(inode[cur_inode_no].direct_block[0], cur_dir);
        for( i = 0; i < MAX_DIR_ENTRY; i++ ){
            if(strcmp(cur_dir->dir_entry[i].name, token) == 0){
                cur_inode_no = cur_dir->dir_entry[i].inode;
                flag = 0;
            }
        }
        if (flag){
            free(cur_dir);
            return -1;
        }
        token = strtok(NULL, "/");
    }
    free(cur_dir);
    return cur_inode_no;
}

int get_dir_inode_no(char *name){
    int cur_inode_no = superBlock.root_dir_inode;
    int previous_inode_no = cur_inode_no;
    Directory *cur_dir;
    cur_dir = create_dir_block();
    int i, flag = 1;
    char* token = strtok(name, "/");
    int token_count = 0;
    while (token) {
        token_count += 1;
        //token here get inode no
        flag = 0;
        read_dir_block(inode[cur_inode_no].direct_block[0], cur_dir);
        for( i = 0; i < MAX_DIR_ENTRY; i++ ){
            if(strcmp(cur_dir->dir_entry[i].name, token) == 0){
                previous_inode_no = cur_inode_no;
                cur_inode_no = cur_dir->dir_entry[i].inode;
                flag = 1;
                break;
            } else{
                flag = 0;
            }

        }
        token = strtok(NULL, "/");
    }
    if ((token_count == 2) && (cur_inode_no == root_directory_inode)){
        return -1;
    }
    if (flag == 1){
        free(cur_dir);
        return previous_inode_no;
    } else {
        free(cur_dir);
        return cur_inode_no;
    }
}


int make_dir(char *name){
    int i;
    char *dir_name = strrchr(name, '/');
    int new_dir_inode;
    int new_dir_block;
    int parent_inode_no = get_dir_inode_no(name);
    if (parent_inode_no == -1){
        printf("ERROR: No such path exist!\n");
        return -1;
    }
    Directory *parent_dir;
    parent_dir = create_dir_block();
    Directory *new_dir;
    read_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
    for(i=0; i<MAX_DIR_ENTRY; i++){
        if(strlen(parent_dir->dir_entry[i].name) == 0){
            new_dir_inode = get_free_inode();
            parent_dir->dir_entry[i].inode = new_dir_inode;
            strcpy(parent_dir->dir_entry[i].name, dir_name+1);
            gettimeofday( &( inode[parent_inode_no].last_modified ), NULL );
            write_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
            break;
        }
    }
    gettimeofday( &( inode[new_dir_inode].last_modified ), NULL );
    new_dir_block = get_free_block();
    inode[new_dir_inode].direct_block[0] = new_dir_block;
    inode[new_dir_inode].size = BLOCK_SIZE;
    inode[new_dir_inode].type = dir;
    inode[new_dir_inode].block_count = 1;
    new_dir = create_dir_block();
    new_dir->dir_entry[0].inode = new_dir_inode;
    strcpy(new_dir->dir_entry[0].name, ".");
    new_dir->dir_entry[1].inode = parent_inode_no;
    strcpy(new_dir->dir_entry[1].name, "..");
    write_dir_block(new_dir_block, new_dir);

    free(parent_dir);
    free(new_dir);
    return new_dir_inode;
}

int remove_dir(char *name){
    int i;
    char *dir_name = strrchr(name, '/');
    int parent_inode_no = get_dir_inode_no(name);
    Directory *parent_dir;
    parent_dir = create_dir_block();
    int removed_dir_inode;
    int removed_dir_block;
    read_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
    for(i=0; i<MAX_DIR_ENTRY; i++){
        if(strcmp(parent_dir->dir_entry[i].name, dir_name+1) == 0){
            removed_dir_inode = parent_dir->dir_entry[i].inode;
            removed_dir_block = inode[removed_dir_inode].direct_block[0];
            set_bit(inode_map, removed_dir_inode, 0);
            superBlock.free_inode_count += 1;
            set_bit(block_map, removed_dir_block, 0);
            superBlock.free_block_count += 1;
            memset(parent_dir->dir_entry[i].name, 0, MAX_FILENAME);
            write_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
            break;
        }
    }
    free(parent_dir);
    return removed_dir_inode;
}

int write_dir_block(int block_no, Directory *buffer){
    if (block_no >= BLOCK_COUNT) {
        return -1;
    }
    int i;

    memcpy(storage + (block_no * BLOCK_SIZE), &(buffer->num_entry), sizeof(int));

    for(i = 0; i < MAX_DIR_ENTRY; ++i) {

        memcpy(storage + (block_no * BLOCK_SIZE) + sizeof(int) + i * sizeof(DirEntry), &(buffer->dir_entry[i]),
               sizeof(DirEntry));
    }

    return 0;
}

int read_dir_block(int block_no, Directory *buffer){
    if (block_no >= BLOCK_COUNT) {
        return -1;
    }
    int i;
    memcpy(&(buffer->num_entry), storage + (block_no * BLOCK_SIZE), sizeof(int));
    for(i = 0; i < MAX_DIR_ENTRY; ++i) {
        memcpy(&buffer->dir_entry[i], storage + (block_no * BLOCK_SIZE) + sizeof(int) + i * sizeof(DirEntry),
               sizeof(DirEntry));
    }
    return 0;

}

int list_dir(char *name){
    int i;
    int inode_no = get_inode_no(name);
    Directory *dir;
    dir = create_dir_block();
    read_dir_block(inode[inode_no].direct_block[0], dir);
    struct tm* ptm;
    char time_string[40];
    for(i=0; i<MAX_DIR_ENTRY; i++){
        if(strlen(dir->dir_entry[i].name) != 0){
            ptm = localtime(&inode[dir->dir_entry[i].inode].last_modified.tv_sec);
            strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
            printf("%d\t", inode[dir->dir_entry[i].inode].size);
            printf("%s\t", time_string);
            if (inode[dir->dir_entry[i].inode].type == 1){
                printf("directory\t");
            } else{
                printf("file     \t");
            }
            printf("%s\n", dir->dir_entry[i].name);
        }
    }
    free(dir);
    return 0;
}

int create_file(char *name){
    int i;
    char *file_name = strrchr(name, '/');
    char name_cpy[255];
    strcpy(name_cpy, name);
    int new_inode_no = get_inode_no(name);
    if (new_inode_no != -1){
        clean_data_blocks(new_inode_no);
        inode[new_inode_no].size = 0;
        inode[new_inode_no].block_count = 0;
        inode[new_inode_no].type = file;
        gettimeofday( &( inode[new_inode_no].last_modified ), NULL );
        return new_inode_no;
    } else {
        new_inode_no = get_free_inode();
        int parent_inode_no = get_dir_inode_no(name_cpy);
        Directory *parent_dir;
        parent_dir = create_dir_block();
        read_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
        for(i=0; i<MAX_DIR_ENTRY; i++){
            if(strlen(parent_dir->dir_entry[i].name) == 0){
                parent_dir->dir_entry[i].inode = new_inode_no;
                strcpy(parent_dir->dir_entry[i].name, file_name+1);
                write_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
                break;
            }
        }
        inode[new_inode_no].type = file;
        inode[new_inode_no].block_count = 0;
        inode[new_inode_no].size = 0;
        gettimeofday( &( inode[new_inode_no].last_modified ), NULL );
        free(parent_dir);
        return new_inode_no;
    }

}

//writes to file
int write_file(char *new_name, char *from_name){
    int i;
    int new_file_inode = create_file(new_name);
    int size, block_count, last_part_size;
    // read file, calculate size and block_count
    FILE *fp = fopen(from_name, "r");
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char *file_content = malloc(size);
    fread(file_content, size, 1, fp);
    block_count = DIV_UP(size, BLOCK_SIZE);
    last_part_size = size % BLOCK_SIZE;
    inode[new_file_inode].size = size;
    inode[new_file_inode].block_count = block_count;
    assign_data_blocks(new_file_inode);
    int offset = 0;
    for(i = 0; i<block_count-1; i++){
        write_block(find_block_no(&inode[new_file_inode], i), file_content+offset, 0);
        offset+=BLOCK_SIZE;
    }
    write_block(find_block_no(&inode[new_file_inode], block_count-1), file_content+offset, last_part_size);
    fclose(fp);
    return 0;
}

int read_file(char *file_name, char *to_name){
    int i;
    int inode_no = get_inode_no(file_name);
    if (inode_no == -1){
        printf("Specified file does not exist!\n");
        return -1;
    }
    FILE *fp = fopen(to_name, "w");
    if (fp == NULL){
        printf("Can not open file!\n");
    }
    int block_count = inode[inode_no].block_count;
    int size = inode[inode_no].size;
    int last_part_size = size % BLOCK_SIZE;
    char *file_content = malloc(size);
    int offset = 0;
    for(i = 0; i<block_count-1; i++){
        read_block(find_block_no(&inode[inode_no], i), file_content+offset, 0);
        offset += BLOCK_SIZE;
    }
    read_block(find_block_no(&inode[inode_no], block_count-1), file_content+offset, last_part_size);
    fwrite(file_content, size, 1, fp);
    fclose(fp);
    return 0;

}


int assign_data_blocks(int inode_no){
    int i,j;
    int block_count = inode[inode_no].block_count;
    //block_count -=1;
    //direct_access
    if (block_count<10){
        for (i = 0; i<block_count;i++){
            inode[inode_no].direct_block[i] = get_free_block();
        }
        return 0;
    }

    //indirect_access
    if (block_count < MAX_TABLE_ENTRY + 10){
        for (i = 0; i<10;i++){
            inode[inode_no].direct_block[i] = get_free_block();
        }


        inode[inode_no].indirect_block = create_table_block();
        int *table = malloc(BLOCK_SIZE);
        read_block(inode[inode_no].indirect_block, (char*) table, 0);
        for (i = 0; i < block_count-10; ++i) {
            table[i] = get_free_block();
        }
        write_block(inode[inode_no].indirect_block, (char*) table, 0);
        free(table);
        return 0;
    }

    //double indirect access
    int index = 0;
    if (block_count < MAX_TABLE_ENTRY*MAX_TABLE_ENTRY + 10){
        int a = DIV_UP((block_count - (MAX_TABLE_ENTRY+10)), MAX_TABLE_ENTRY);
        int k = (block_count - (MAX_TABLE_ENTRY+10));
        int m = MAX_TABLE_ENTRY;
        int b = k % m;

        for (i = 0; i<10;i++){
            inode[inode_no].direct_block[i] = get_free_block();

            index+=1;
        }

        inode[inode_no].indirect_block = create_table_block();
        int *table = malloc(BLOCK_SIZE);
        read_block(inode[inode_no].indirect_block, (char*) table, 0);
        for (i = 0; i < MAX_TABLE_ENTRY; ++i) {
            table[i] = get_free_block();
            index+=1;
        }
        write_block(inode[inode_no].indirect_block, (char*) table, 0);

        inode[inode_no].double_indirect_block = create_table_block();
        read_block(inode[inode_no].double_indirect_block, (char*) table, 0);

        for (i = 0; i < a; i++) {
            table[i] = create_table_block();
        }
        write_block(inode[inode_no].double_indirect_block, (char*) table, 0);
        int *table2 = malloc(BLOCK_SIZE);
        for ( i = 0; i < a-1; ++i) {
            read_block(table[i], (char*) table2, 0);
            for (j = 0; j < MAX_TABLE_ENTRY; ++j) {
                table2[j] = get_free_block();
                index+=1;
            }
            write_block(table[i], (char*) table2, 0);
        }
        read_block(table[a-1], (char*) table2, 0);
        for (j = 0; j < b+1 ; j++) {
            table2[j] = get_free_block();
            index+=1;
        }
        write_block(table[a-1], (char*) table2, 0);
        free(table);
        free(table2);
        return 0;
    }

}

int find_block_no(Inode *inode, int block_count){
    int block_no;
    if (block_count < 10) {
        return inode->direct_block[block_count];
    }

    int *table = malloc(BLOCK_SIZE);
    if (block_count < MAX_TABLE_ENTRY + 10) {
        read_block(inode->indirect_block, (char*) table, 0);
        return table[block_count - 10];
    }
    int *table2 = malloc(BLOCK_SIZE);
    int k = (block_count - (MAX_TABLE_ENTRY+10));
    int m = MAX_TABLE_ENTRY;
    int i = k / m;
    int a = (block_count - (MAX_TABLE_ENTRY+10));
    int b = MAX_TABLE_ENTRY;
    int j = a % b;

    read_block(inode->double_indirect_block, (char*) table, 0);
    // find secont table location in first table
    read_block(table[i], (char*) table2, 0);
    block_no = table2[j];
    free(table);
    free(table2);
    return block_no;
}

int clean_data_blocks(int inode_no){
    int i;
    int block_count = inode[inode_no].block_count;
    for (i = 0; i < block_count; ++i) {
        set_bit(block_map, find_block_no(&inode[inode_no], i), 0);
        superBlock.free_block_count += 1;
    }
    if(block_count < 10){
        for (i = 0; i < block_count; ++i) {
            set_bit(block_map, inode[inode_no].direct_block[i], 0);
        }
        return 0;
    } else if (block_count < MAX_TABLE_ENTRY+10){
        set_bit(block_map, inode[inode_no].indirect_block, 0);
        superBlock.free_block_count += 1;
        return 0;
    } else {
        int table_count = DIV_UP((block_count - (MAX_TABLE_ENTRY+10)), MAX_TABLE_ENTRY);
        int *table = malloc(BLOCK_SIZE);
        set_bit(block_map, inode[inode_no].indirect_block, 0);
        superBlock.free_block_count += 1;
        read_block(inode[inode_no].double_indirect_block, (char*) table, 0);
        for (i = 0; i < table_count; i++) {
            set_bit(block_map, table[i], 0);
            superBlock.free_block_count += 1;
        }
        set_bit(block_map, inode[inode_no].double_indirect_block, 0);
        superBlock.free_block_count += 1;
        free(table);
        return 0;
    }

}

int delete_file(char *name){
    int i;
    char name_cpy[255];
    strcpy(name_cpy, name);
    char *file_name = strrchr(name, '/');
    int deleted_inode_no = get_inode_no(name);
    if (deleted_inode_no == -1){
        printf("Specified file does not exist!\n");
    }
    clean_data_blocks(deleted_inode_no);
    int parent_inode_no = get_dir_inode_no(name_cpy);
    Directory *parent_dir;
    parent_dir = create_dir_block();
    read_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
    for(i=0; i<MAX_DIR_ENTRY; i++){
        if(strcmp(parent_dir->dir_entry[i].name, file_name+1) == 0){
            memset(parent_dir->dir_entry[i].name, 0, MAX_FILENAME);
            write_dir_block(inode[parent_inode_no].direct_block[0], parent_dir);
            break;
        }
    }
    set_bit(inode_map, deleted_inode_no, 0);
    superBlock.free_inode_count += 1;
    free(parent_dir);
    return deleted_inode_no;
}

int list_dir_by_inode(int inode_no){
    int i;
    Directory *dir;
    dir = create_dir_block();
    read_dir_block(inode[inode_no].direct_block[0], dir);
    for (i = 0; i<MAX_DIR_ENTRY; i++){
        if(  (strlen(dir->dir_entry[i].name) != 0) && ( (strcmp(dir->dir_entry[i].name, ".") !=0) && (strcmp(dir->dir_entry[i].name, "..")!=0) ) ){
            printf("%-7d", dir->dir_entry[i].inode);
            printf("%-20s", dir->dir_entry[i].name);
            if (inode[dir->dir_entry[i].inode].type == 0){
                printf("file         ");
            } else{
                printf("directory    ");
            }
            printf("%d\n", inode[dir->dir_entry[i].inode].block_count);

        }
    }
    return 0;
}





int simple_dumpe2fs(){
    int i;
    printf("Block Size: %d\n", BLOCK_SIZE);
    printf("Block Count: %d\n", BLOCK_COUNT);
    printf("Inode Count: %d\n", INODE_COUNT);
    printf("Free Block Count: %d\n", superBlock.free_block_count);
    printf("Free Inode Count: %d\n", superBlock.free_inode_count);
    // print root inode
    printf("Inode  Name                Type         Number of Block\n");
    printf("%-7d", superBlock.root_dir_inode);
    printf("/                   ");
    printf("directory    ");
    printf("%d\n", inode[root_directory_inode].block_count);

    for(i = 0; i<INODE_COUNT; i++){
        if (get_bit(inode_map, i) == 1){
            if (inode[i].type == dir){
                list_dir_by_inode(i);
            }
        }
    }


    return 0;
}

int get_command_no(char *name){
    if(strcmp(name, "list") == 0){
        return 0;
    } else if(strcmp(name, "mkdir") == 0){
        return 1;
    } else if(strcmp(name, "rmdir") == 0){
        return 2;
    }else if(strcmp(name, "dumpe2fs") == 0){
        return 3;
    }else if(strcmp(name, "write") == 0){
        return 4;
    }else if(strcmp(name, "read") == 0){
        return 5;
    }else if(strcmp(name, "del") == 0){
        return 6;
    }
    return -1;
}

int list_inode(){
    int i;
    for (i = 0; i < INODE_COUNT; ++i) {
        if (get_bit(inode_map, i) == 1){
            printf("inde_no: %d, direct_block: %d\n", i, inode[i].direct_block[0]);
        }
    }
}