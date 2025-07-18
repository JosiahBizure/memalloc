#include "memalloc.h"

void* malloc(size_t size) {
    size_t total_size;
    void* block;
    header_t* header;

    // If the requested size is zero, return NULL
    if (size == NULL) return NULL;

    /*
        For valid sizes, first we aquire the lock
        then we call get_free_block()
    */
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
    if (header != NULL) { // Free block found!
        header->s.is_free = 0; // mark the block as allocated
        pthread_mutex_unlock(&global_malloc_lock);

        // Return pointer to memory immediately after header
        return (void*)(header + 1);
    }

    /*
        If we have not found a sufficiently large free block
        then we have to extend the heap by calling sbrk()
    */
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = (header_t*)block; // Explicitly cast void* to header_t* for clarity
    header->s.size = size; // Using requested size (not total size)
    header->s.is_free = 0; // Mark the block as not free

    // Update the next, head, and tail pointers to reflect the new state of the list
    header->s.next = NULL;

    /*
        If this is the first block, set head.
        Otherwise, link it to the end of the list.
    */
    if (head == NULL) {
        head = header;
    } else {
        tail->s.next = header;
    }

    tail = header;

    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1); // Again, pointer to the actual memory block
}

header_t* get_free_block(size_t size) {
    header_t* curr = head;
    while (curr) {
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
            
        curr = curr->s.next;
    }
    return NULL;
}

void free(void* block) {
    header_t* header, *tmp;
    void* program_break;

    if (block == NULL) return; // Memory is already unallocated
    pthread_mutex_lock(&global_malloc_lock);

    // Move back one header-sized unit to access the block's metadata
    header = (header_t*)block - 1;

    // If the block is already free no need to continue
    if (header->s.is_free) {
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    program_break = sbrk(0);

    /*
        If the target block is at the end / top of the heap, we release the
        memory back to the operating system
    */
    if ((char*)block + header->s.size == program_break) {
        // If target block is the only element in the list
        // set head and tail to NULL
        if (head == tail) {
            head = tail = NULL;
        } else {
            // If there are other blocks, set tail to the block previous to the target block
            tmp = head;
            while (tmp != NULL) {
                if (tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                    break; // Stop traversing once we updte the tail
                }
                tmp = tmp->s.next;
            }
        }

        // Shrink the heap by the size of the block + its header
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    // If the target block is not the last block, simply mark it as free and return
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}