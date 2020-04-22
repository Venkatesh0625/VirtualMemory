#ifndef DISPLAY_C
#define DISPLAY_C

#include <stdio.h>

#include "memory.h"
#include "stats.h"

void display_stats(struct stat* statistic)
{
    
    double pf_rate  = statistic->page_faults / (double) statistic->n_read;
    double TLB_rate = statistic->TLB_hits / (double) statistic->n_read;
    printf("\n\nNumber of translated addresses : %d\n\n", statistic->n_read);
    printf("Page Faults \t: %d\n\n", statistic->page_faults);
    printf("Page Fault Rate : %.3f\n\n", pf_rate);
    printf("TLB Hits \t: %d\n\n", statistic->TLB_hits);
    printf("TLB Hit Rate \t: %.3f\n\n", TLB_rate);
}

void display_output(struct memory* data)
{
    
    printf("Virtual address\t: %d \n\nPhysical address: %d", data->logical_address, (data->frame_number << 8) | data->offset);
    
    printf("\n\nFrame number\t: %d\n\n", data->frame_number);
    
    printf("data->offset\t: %d\n\n", data->offset);

    printf("Value: %d\n\n", data->current_value);
}

#endif 