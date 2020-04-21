#ifndef DATA_H
#define DATA_H

#include  <stdio.h>

#define FRAME_SIZE 256
#define MAX_FRAMES 256
#define TLB_CACHE_SIZE 16
#define PAGETABLE_SIZE 256

struct memory
{
    int page_table[PAGETABLE_SIZE][2];  
    
    int TLB_cache[TLB_CACHE_SIZE][2]; 

    int physical_memory[MAX_FRAMES][FRAME_SIZE]; 

    //currently loaded logical address
    int logical_address;
    
    //stores current value from physical memory
    signed char current_value;

    int page_number;

    int offset;

    int frame_number;
};

void init_memory(struct memory* data)
{
    data->current_value = 0;
    data->logical_address = 0;
}

#endif

