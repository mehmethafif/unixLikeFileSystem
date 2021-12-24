//
// Created by mehmet on 24.05.2020.
//

#include "bitmap.h"
#include "file_system.h"


void toggle_bit( char * array, int index ){
    array[index/8] ^= 1 << ( index % 8 );
}


char get_bit( char * array, int index ){
    return 1 & ( array[index/8] >> ( index % 8 ));
}

void set_bit( char * array, int index, char value ){
    if( value != 0 && value != 1 ){
        return;
    }
    //printf( "SET BIT %d\n", array[index/8] ^= 1 << ( index % 8 ));
    array[index/8] ^= 1 << ( index % 8 );
    if( get_bit( array, index ) == value){
        return;
    }
    toggle_bit( array, index );
}

int get_free_inode(){
    int i = 0;
    for( i = 0; i < INODE_COUNT; i++ ){
        if( get_bit( inode_map, i ) == 0){
            set_bit( inode_map, i, 1 );
            superBlock.free_inode_count--;
            return i;
        }
    }
    return -1;
}

int get_free_block(){
    int i = 0;
    for( i = 0; i < BLOCK_COUNT; i++ ){
        if( get_bit( block_map, i ) == 0 ){
            set_bit( block_map, i, 1 );
            superBlock.free_block_count--;
            return i;
        }
    }
    return -1;
}