#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>

#define FRAME_SIZE 256        // size of the frame
#define TOTAL_NUMBER_OF_FRAMES 256  // total number of frames in physical memory
#define ADDRESS_MASK  0xFFFF  //address mask
#define OFFSET_MASK  0xFF
#define TLB_SIZE 16       // size of the TLB
#define PAGE_TABLE_SIZE 256  // size of the page table
#define CHUNK_SIZE 256

#define PAGE 0
#define FRAME 1

unsigned int page_table[2][PAGE_TABLE_SIZE];

unsigned int TLB_cache[2][TLB_SIZE];

unsigned int physical_memory[TOTAL_NUMBER_OF_FRAMES][FRAME_SIZE];

char buffer[CHUNK_SIZE];

unsigned int page_fault = 0;
unsigned int TLB_hits = 0;
unsigned int pagetable_index = 0;
unsigned int current_TLB_size = 0;
unsigned int frame_index = 0;

FILE *address_file;
FILE *output_file;

char address[10];
int logical_address;

signed char value;

void get_page(unsigned int address);
void read_from_store(unsigned int page_number);
void insert_into_TLB(unsigned int page_number,int frame_number);

void get_page(unsigned int logical_address) {

    //Obtaining page number and offset value from the given logical address
    int page_number = ((logical_address & ADDRESS_MASK) >> 8);
    int offset = (logical_address & OFFSET_MASK);

    //Try to get page from TLB , initialising with -1
    int frame_number = -1;

    int i;
    for(i = 0; i < TLB_SIZE ; i++) {
        //if the TLB index is equal to the page number 
        if(TLB_cache[PAGE][i] == page_number) 
        {
            //then frame is stored in frame_number
            frame_number = TLB_cache[FRAME][i];
            //update hits of TLB;
            TLB_hits++;
        }
    }

    //if it doesn't hits in cache 
    if(!(frame_number+1))
    {
        int i; 
        //iterate over page table
        for(i = 0; i < pagetable_index; i++)
            //if found in page table 
            if(page_table[PAGE][i] == page_number)
                //extracts the frame number and stored
                frame_number = page_table[FRAME][i];
        
        //if after that also we didnt hit it 
        if(!(frame_number+1))
        {
            //load it from store or secondary memory
            read_from_store(page_number);
            //increment count of page faults
            page_fault++;
            //the frame numbe rallocated to it will be first avilable frame index
            frame_number = pagetable_index - 1;
        }
    }

    insert_into_TLB(page_number,frame_number);
    value = physical_memory[frame_number][offset];
    printf("frame number : %d\n",frame_number);
    printf("Offset : %d\n",offset);
    printf("Virtual address : %d Physical address : %d Value : %d \n",logical_address,(frame_number <<8) | offset,value);

}

void insert_into_TLB(unsigned int page_number,unsigned int frame_number)
{
    int i;
    //if it's already in cache
    for(i = 0; i < current_TLB_size; i++)
        if(TLB_cache[PAGE][i] == page_number)
            break;
    
    //if number of entries is equal to the index
    if(i == current_TLB_size)
    {
        //if it has still space in it 
        if(current_TLB_size < TLB_SIZE)
        {
            TLB_cache[PAGE][current_TLB_size] = page_number;
            TLB_cache[FRAME][current_TLB_size] = frame_number;
        }
        //else move everything one index before 
        //FIFO cache 
        else
        {
            for(i = 0;i < TLB_SIZE - 1; i++)
            {
                TLB_cache[PAGE][i] = TLB_cache[PAGE][i+1];
                TLB_cache[FRAME][i] = TLB_cache[FRAME][i+1];
            }
        }
        TLB_cache[PAGE][current_TLB_size-1] = page_number;
        TLB_cache[FRAME][current_TLB_size-1] = frame_number; 
    }
    else
    {
        for(i = i;i < current_TLB_size - 1; i++)
        {
            TLB_cache[PAGE][i] = TLB_cache[PAGE][i+1];
            TLB_cache[FRAME][i] = TLB_cache[FRAME][i+1];
        }
        if(current_TLB_size < TLB_SIZE)
        {
            TLB_cache[PAGE][current_TLB_size] = page_number;
            TLB_cache[FRAME][current_TLB_size] = frame_number;
        }
        else
        {
            TLB_cache[PAGE][current_TLB_size-1] = page_number;
            TLB_cache[FRAME][current_TLB_size-1] = frame_number;
        }
    }
    if(current_TLB_size < TLB_SIZE)
        current_TLB_size++;

}

void read_from_store(int page_number)
{
    int i;
    if(fseek(output_file,page_number * 256,SEEK_SET) != 0)
        fprintf(stderr,"Error seeking in output file\n");

    if(fread(buffer,sizeof(signed char),256,backing_store) == 0)
        fprintf(stderr, "Error reading from backing store\n");

    for(i = 0; i < CHUNK; i++)
        physical_memory[frame_index] = page_number;

    page_table[PAGE][pagetable_index] = page_number;
    page_table[FRAME][pagetable_index] = frame_index;

    frame_index++;
    pagetable_index++; 

}

int main(int argc,char **argv)
{
    int (argc != 2) {
        fprintf(stderr,"Usage : ./program [input file]\n");
        return -1;
    }

    backing_store = fopen("BACKING_STORE.bin"."rb");

    if(backing_store == NULL)
    {
        fprintf(stderr,"Error opening BACKING_STORE.bin %s\n");
        return -1;
    }

    address_file = fopen(argv[1],"r");

    if(address_file == NULL)
    {
        fprintf("Error opening address file\n");
        return -1;
    }

    int number_of_translated_address = 0;

    while(fgets(address, BUFFER_SIZE, address_file) != NULL)
    {
        logical_address = atoi(address);
        get_page(logical_address);
        number_of_translated_address++;
    }

    printf("Number of translated addresses = %d\n",number_of_translated_address);
    double pf_rate = page_fault / (double)number_of_translated_address;
    double TLB_rate = TLB_hits / (double)number_of_translated_address;

    printf("Page faults = %d\n",page_fault);
    printf("Page Fault rate = %f\n",pf_rate);
    printf("TLB Hit rate = %f\n",TLB_rate);

    fclose(address_file);
    fclose(backing_store); 

    return 0;

}