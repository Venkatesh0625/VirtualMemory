#ifndef VIRTUAL_C
#define VIRTUAL_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"
#include "stats.h"
#include "display.c"


#define ADDRESS_MASK 0xFFFF
#define OFFSET_MASK 0xFF
#define BUFFER_SIZE 10
#define CHUNK 256

//Function prototypes
void get_page(struct memory*, struct stat*,FILE*);
void fetch_secondaryMemory(struct memory*, struct stat*,FILE*);
void update_cache(struct memory*, struct stat*);

//Function to get the page from either one of the source 
void get_page(struct memory* data, struct stat* statistic, FILE* backing_store) 
{
    data->page_number = ((data->logical_address & ADDRESS_MASK) >> 8);
    data->offset = (data->logical_address & OFFSET_MASK);

    data->frame_number = -1; 

    int i;
    
    for (i = 0; i < TLB_CACHE_SIZE; i++) 
    {
        if (data->TLB_cache[i][0] == data->page_number) 
        { 
            data->frame_number = data->TLB_cache[i][1]; 
            statistic->TLB_hits++;
        }
    }

    if (data->frame_number == -1) {
        int i; 
        for (i = 0; i < statistic->pageTable_index; i++) {
            if (data->page_table[i][0] == data->page_number) { 
                data->frame_number = data->page_table[i][1]; 
            }
        }
        if (data->frame_number == -1) 
        { 
            fetch_secondaryMemory(data, statistic, backing_store); 
            statistic->page_faults++; 
            data->frame_number = statistic->frame_index - 1; 
        }
    }

    update_cache(data, statistic); 

    data->current_value = data->physical_memory[data->frame_number][data->offset]; 

    display_output(data);
}

// function to insert a page number and frame number into the TLB with a FIFO replacement
void update_cache(struct memory* data,struct stat* statistic) {

    int i; 
    
    for (i = 0; i < statistic->TLB_count; i++) 
        if (data->TLB_cache[i][0] == data->page_number) 
            break;

    if (i == statistic->TLB_count) 
        if (statistic->TLB_count < TLB_CACHE_SIZE) 
        { 
            data->TLB_cache[statistic->TLB_count][0] = data->page_number; 
            data->TLB_cache[statistic->TLB_count][1] = data->frame_number;
        }
        
        else 
        { 
            for (i = 0; i < TLB_CACHE_SIZE - 1; i++) 
            {
                data->TLB_cache[i][0] = data->TLB_cache[i + 1][0];
                data->TLB_cache[i][1] = data->TLB_cache[i + 1][1];
            }
            data->TLB_cache[statistic->TLB_count - 1][0] = data->page_number; 
            data->TLB_cache[statistic->TLB_count - 1][1] = data->frame_number;
        }

    // if the index is not equal to the number of entries
    else 
    {
        for (i = i; i < statistic->TLB_count - 1; i++) 
        { 
            data->TLB_cache[i][0] = data->TLB_cache[i + 1][0];
            data->TLB_cache[i][1] = data->TLB_cache[i + 1][1];
        }

        if (statistic->TLB_count < TLB_CACHE_SIZE) 
        { 
            data->TLB_cache[statistic->TLB_count][0]= data->page_number;
            data->TLB_cache[statistic->TLB_count][1] = data->frame_number;
        } 
        
        else 
        { 
            data->TLB_cache[statistic->TLB_count - 1][0] = data->page_number;
            data->TLB_cache[statistic->TLB_count - 1][1] = data->frame_number;
        }
    }

    if (statistic->TLB_count < TLB_CACHE_SIZE) 
        statistic->TLB_count++;
}

// function to read from the backing store and bring the frame into physical memory and the page table
void fetch_secondaryMemory(struct memory* data, struct stat* statistic,FILE* backing_store) 
{    
    char address[BUFFER_SIZE];
    signed char buffer[CHUNK];
    
    if (fseek(backing_store, data->page_number * CHUNK, SEEK_SET) != 0) 
    {
        fprintf(stderr, "Error seeking in backing store\n");
    }

    if (fread(buffer, sizeof(signed char), CHUNK, backing_store) == 0) 
    {
        fprintf(stderr, "Error reading from backing store\n");
    }

    int i;
    for (i = 0; i < CHUNK; i++) 
    {
        data->physical_memory[statistic->frame_index][i] = buffer[i];
    }

    data->page_table[statistic->pageTable_index][0] = data->page_number;
    data->page_table[statistic->pageTable_index][1] = statistic->frame_index;

    statistic->frame_index++;
    statistic->pageTable_index++;
}

#endif