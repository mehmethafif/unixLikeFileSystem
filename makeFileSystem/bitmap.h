//
// Created by mehmet on 24.05.2020.
//

#ifndef OS_MIDTERM_PART1_BITMAP_H
#define OS_MIDTERM_PART1_BITMAP_H


char get_bit( char * array, int index );
void set_bit( char * array, int index, char value );
int get_free_inode();
int get_free_block();

#endif //OS_MIDTERM_PART1_BITMAP_H