# Heap Allocator Implementation in C

This implementation of a heap allocator in C follows a simple approach:

1. **Heap Initialization**:
   It first requests a 4096-byte region to be used as the heap.

   ```
   [                                                                ]
   ```

2. **Metadata Storage**:
   The heap region is split up to store metadata such as size.

   ```
   [info][head][                                                     ]
   ```

   Info Chunk Raw Memory

3. **Memory Allocation**:
   When a user allocates memory, the chunk is split up into smaller chunks.

   ```
   [info][head][          ][head2][                                  ]
   ```

   This allocated memory is then returned.

4. **Memory Deallocation**:
   When a user frees memory, the `inuse` flag is set to false. To avoid fragmentation, unused chunks are merged together.

   ```
   [info][head][          ][head2][                                  ]
   ```

   Is then merged to:

   ```
   [info][head][                                                     ]
   ```

Note: For searching through usable chunks, the size is added to the original pointer.

```
[info][ptrA][          ][ptrB][                                   ]
```

By adding `sizeof(HeapChunk)` and `chunk->size`, we can go to the next chunk without using a linked list.
However, this means that the chunks have to be arranged linearly and without gaps.
