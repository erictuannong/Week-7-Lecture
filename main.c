#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

#define HEAP_SIZE 400
#define VAL_SIZE 8

uint64_t* HEAP_START = NULL;

void init_heap() {
    // Hey operating system, where can I start my heap
    // mmap means "hey OS, I want heap memory, give me a bunch and tell me where it started"
    uint64_t* heap = mmap(NULL, 
                          HEAP_SIZE, 
                          PROT_READ | PROT_WRITE, 
                          MAP_ANON | MAP_SHARED, 
                          -1, 
                          0);
    HEAP_START = heap;
    // more setup?
    *HEAP_START = HEAP_SIZE - VAL_SIZE;
}

void* my_malloc(size_t size) {
    uint64_t* current = HEAP_START;
    while (current < (HEAP_START + (HEAP_SIZE / VAL_SIZE))) {
        uint64_t cur_header = *current;
        uint64_t cur_size = (cur_header / 2) * 2;
        // want to know (a) size (b) is it free
        if (cur_header % 2 == 0 && size <= cur_size) { // check if free
            // GOAL: split up the block into the malloc'd part and the free part
            // Round up size to next multiple of 8
            size_t rounded = ((size + 7) / 8) * 8;
            *current = rounded + 1; // rounds up and sets the "busy" bit

            // what if remaining is close to 0/8/16, etc.
            size_t remaining = cur_size - (rounded + VAL_SIZE);
            uint64_t* remaining_ptr = current + (rounded / VAL_SIZE) + 1;
            *remaining_ptr = remaining;
            return current + 1;
        }
        else {
            uint64_t* next = current + (cur_size / VAL_SIZE) + 1;
            current = next;
        }
    }
    return NULL;
}

int main() {
    init_heap();
    //printf("%lu %lu\n", HEAP_START[0], HEAP_START[1]);
    int* a = my_malloc(40);
    printf("%llu %llu %llu\n", HEAP_START[0], HEAP_START[1], HEAP_START[6]);

    int* b = my_malloc(10);
    printf("%llu %llu\n", HEAP_START[6], HEAP_START[9]);

    int* c = my_malloc(321);
    printf("%p\n", c);
}