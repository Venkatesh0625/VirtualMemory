#ifndef STATS_H
#define STATS_H

#include <stdio.h>

struct stat
{
    short int page_faults;
    short int TLB_hits; 
    short int frame_index;
    short int pageTable_index;
    short int TLB_count; 
    short int n_read;
};

void init_stat(struct stat* statistic)
{
    statistic->page_faults = 0;
    statistic->TLB_hits = 0;
    statistic->frame_index = 0;
    statistic->pageTable_index = 0;
    statistic->TLB_count = 0;
    statistic->n_read = 0;
}

#endif