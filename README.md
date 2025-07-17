# memalloc

A simple heap memory allocator in C, demonstrating how `malloc`, `calloc`, `realloc`, and `free` work under the hood.

This project is intended for educational purposes and helps develop an understanding of:
- Process memory layout (stack, heap, .text, .data, .bss)
- Dynamic memory management
- Low-level system calls (`sbrk`)
- Linked list-based block tracking
- Memory alignment and fragmentation handling

## Features

- Custom implementation of `malloc(size_t size)`
  - Allocates heap memory using `sbrk`
  - Uses a 16-byte aligned header to track block size and allocation status
  - Stores blocks in a linked list for reuse (first-fit strategy)
- Basic free block reuse (no coalescing yet)
- Thread-safety using a global `pthread_mutex_t`
- 16-byte alignment for all returned memory

## Header Structure

Each block of allocated memory includes a hidden header before the returned pointer:

```c
union header {
    struct {
        size_t size;           // size of the user block
        unsigned int is_free;  // flag indicating if block is free
        union header* next;    // pointer to next block in the list
    } s;
    char align[16];            // ensure 16-byte alignment
};
