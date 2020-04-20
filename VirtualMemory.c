#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define frame_size 256
#define total_frames 256
#define address_mask 0xFFFF
#define offset_mask 0xFF
#define TLB_size 16
#define pageTable_size 256


int page_table[pageTable_size][2];  

int TLB_cache[TLB_size][2]; 

int physical_memory[total_frames][frame_size]; // physical memory 2D array

int page_faults = 0; // counter to track page faults
int TLB_hits = 0; // counter to track TLB hits
int frame_index = 0; // counter to track the first available frame
int pageTable_index = 0; // counter to track the first available page table entry
int TLB_count = 0; // counter to track the number of entries in the TLB

// number of characters to read for each line from input file
#define BUFFER_SIZE 10

// number of bytes to read
#define CHUNK 256

FILE * address_file;
FILE * backing_store;

// how we store reads from input file
char address[BUFFER_SIZE];
int logical_address;

// the buffer containing reads from backing store
signed char buffer[CHUNK];

// the value of the byte (signed char) in memory
signed char value;


void get_page(int address);
void readFromStore(int page_number);
void insertIntoTLB(int page_number, int frame_number);

void get_page(int logical_address) {

    int page_number = ((logical_address & address_mask) >> 8);
    int offset = (logical_address & offset_mask);

    int frame_number = -1; 

    int i;
    
    for (i = 0; i < TLB_size; i++) {
        if (TLB_cache[i][0] == page_number) { // if the TLB index is equal to the page number
            frame_number = TLB_cache[i][1]; // then the frame number is extracted
            TLB_hits++; // and the TLBHit counter is incremented
        }
    }

    if (frame_number == -1) {
        int i; 
        for (i = 0; i < pageTable_index; i++) {
            if (page_table[i][0] == page_number) { // if the page is found in those contents
                frame_number = page_table[i][1]; // extract the frame_number from its twin array
            }
        }
        if (frame_number == -1) { // if the page is not found in those contents
            readFromStore(page_number); // page fault, call to readFromStore to get the frame into physical memory and the page table
            page_faults++; // increment the number of page faults
            frame_number = frame_index - 1; // and set the frame_number to the current frame_index index
        }
    }

    insertIntoTLB(page_number, frame_number); // call to function to insert the page number and frame number into the TLB
    value = physical_memory[frame_number][offset]; // frame number and offset used to get the signed value stored at that address
    printf("frame number: %d\n", frame_number);
    printf("offset: %d\n", offset);
    // output the virtual address, physical address and value of the signed char to the console
    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frame_number << 8) | offset, value);
}

// function to insert a page number and frame number into the TLB with a FIFO replacement
void insertIntoTLB(int page_number, int frame_number) {

    int i; 
    // if it's already in the TLB, break
    for (i = 0; i < TLB_count; i++) {
        if (TLB_cache[i][0] == page_number) {
            break;
        }
    }

    // if the number of entries is equal to the index
    if (i == TLB_count) {
        if (TLB_count < TLB_size) { // and the TLB still has room in it
            TLB_cache[TLB_count][0] = page_number; // insert the page and frame on the end
            TLB_cache[TLB_count][1] = frame_number;
        } else { // otherwise move everything over
            for (i = 0; i < TLB_size - 1; i++) {
                TLB_cache[i][0] = TLB_cache[i + 1][0];
                TLB_cache[i][1] = TLB_cache[i + 1][1];
            }
            TLB_cache[TLB_count - 1][0] = page_number; // and insert the page and frame on the end
            TLB_cache[TLB_count - 1][1] = frame_number;
        }
    }

    // if the index is not equal to the number of entries
    else {
        for (i = i; i < TLB_count - 1; i++) { // iterate through up to one less than the number of entries
            TLB_cache[i][0] = TLB_cache[i + 1][0]; // move everything over in the arrays
            TLB_cache[i][1] = TLB_cache[i + 1][1];
        }
        if (TLB_count < TLB_size) { // if there is still room in the array, put the page and frame on the end
            TLB_cache[TLB_count][0]= page_number;
            TLB_cache[TLB_count][1] = frame_number;
        } else { // otherwise put the page and frame on the number of entries - 1
            TLB_cache[TLB_count - 1][0] = page_number;
            TLB_cache[TLB_count - 1][1] = frame_number;
        }
    }
    if (TLB_count < TLB_size) { // if there is still room in the arrays, increment the number of entries
        TLB_count++;
    }
}

// function to read from the backing store and bring the frame into physical memory and the page table
void readFromStore(int page_number) {
    // first seek to byte CHUNK in the backing store
    // SEEK_SET in fseek() seeks from the beginning of the file
    if (fseek(backing_store, page_number * CHUNK, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking in backing store\n");
    }

    // now read CHUNK bytes from the backing store to the buffer
    if (fread(buffer, sizeof(signed char), CHUNK, backing_store) == 0) {
        fprintf(stderr, "Error reading from backing store\n");
    }

    // load the bits into the first available frame in the physical memory 2D array
    int i;
    for (i = 0; i < CHUNK; i++) {
        physical_memory[frame_index][i] = buffer[i];
    }

    // and then load the frame number into the page table in the first available frame
    page_table[pageTable_index][0] = page_number;
    page_table[pageTable_index][1] = frame_index;

    // increment the counters that track the next available frames
    frame_index++;
    pageTable_index++;
}

// main opens necessary files and calls on get_page for every entry in the addresses file
int main(int argc, char * argv[]) {
    // perform basic error checking
    if (argc != 2) {
        fprintf(stderr, "Usage: ./a.out [input file]\n");
        return -1;
    }

    // open the file containing the backing store
    backing_store = fopen("BACKING_STORE.bin", "rb");

    if (backing_store == NULL) {
        fprintf(stderr, "Error opening BACKING_STORE.bin %s\n", "BACKING_STORE.bin");
        return -1;
    }

    // open the file containing the logical addresses
    address_file = fopen(argv[1], "r");

    if (address_file == NULL) {
        fprintf(stderr, "Error opening addresses.txt %s\n", argv[1]);
        return -1;
    }

    int numberOfTranslatedAddresses = 0;
    // read through the input file and output each logical address
    while (fgets(address, BUFFER_SIZE, address_file) != NULL) {
        logical_address = atoi(address);

        // get the physical address and value stored at that address
        get_page(logical_address);
        numberOfTranslatedAddresses++; // increment the number of translated addresses        
    }

    // calculate and print out the stats
    printf("Number of translated addresses = %d\n", numberOfTranslatedAddresses);
    double pfRate = page_faults / (double) numberOfTranslatedAddresses;
    double TLBRate = TLB_hits / (double) numberOfTranslatedAddresses;

    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", pfRate);
    printf("TLB Hits = %d\n", TLB_hits);
    printf("TLB Hit Rate = %.3f\n", TLBRate);

    // close the input file and backing store
    fclose(address_file);
    fclose(backing_store);

    return 0;
}