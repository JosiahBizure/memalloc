#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>     // size_t, NULL
#include <pthread.h>    // pthread_mutex_t

typedef char ALIGN[16];

union header {
    struct {
        size_t size;
        unsigned int is_free;
        header_t* next;
    } s;
    ALIGN stub;
};
typedef union header header_t;

header_t* head, *tail; // Used for traversing the memory blocks
pthread_mutex_t global_malloc_lock; // Concurruent memory access prevention

/*
    Allocates 'size' bytes of heap memory
    returns a pointer to the allocated block
*/
void* malloc(size_t size);

/*
    Traverse the linked list and see if there exists a large enough free
    block of memory for a requested size.
    Returns the address of first valid block found (first-fit approach)
*/
header_t* get_free_block(size_t size);


void free(void* block); // Deallocates previously allocated memory

// Allocates memory ofr an array of num elements of nsize bytes each
// Additionally, zeroes out all memory
void* calloc(size_t num, size_t nsize);

// Changes the size of the givne memory block to the requested size
void* realloc(void* block, size_t size);


#endif // MEMALLOC_H