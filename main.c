#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"
#include "stats.h"
#include "VirtualMemory.c"
#include "display.c"

int main(int argc, char * argv[]) {
    
    char address[10];

    if (argc != 2) {
        printf("Run File like : ./<executable> <input address file>\n");
        return EXIT_FAILURE;
    }

    FILE* address_file;
    FILE* backing_store;

    struct memory* data = (struct memory*) malloc(sizeof(struct memory));
    init_memory(data);

    struct stat* statistic = (struct stat*) malloc(sizeof(struct stat));
    init_stat(statistic);
    
    
    // open the file containing the backing store
    backing_store = fopen("BACKING_STORE.bin", "rb");

    if (backing_store == NULL) 
    {
        printf("Error opening BACKING_STORE.bin %s\n", "BACKING_STORE.bin");
        return EXIT_FAILURE;
    }

    // open the file containing the logical addresses
    address_file = fopen(argv[1], "r");

    if (address_file == NULL) 
    {
        printf("Error opening addresses.txt %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    while (fgets(address, BUFFER_SIZE, address_file) != NULL) 
    {
        data->logical_address = atoi(address);

        get_page(data, statistic, backing_store);
        statistic->n_read++;
        usleep(150000);
        system("clear");
    }

    // Print out the stats
    display_stats(statistic);

    // close the input file and backing store
    fclose(address_file);
    fclose(backing_store);

    return 0;
}