This project implments a simple bump allocator in C.

It contains just a single function: `heap_alloc` which bumps up the pointer to allocate new memory.
Bump allocators are not able to free up memory individually so a `heap_reset` function is used to reset the heap.
