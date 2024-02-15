## Implments a heap allocator in C.

it first asks for a 4096 byte region that will be used for the heap.
`[                                                                 ]`

it is then split up for storing metadata such as size.
`[   ][     ][                                                     ]`
info chunk raw memory

when a user allocates memory, the chunk is then split up into smaller chunks;

`[   ][     ][          ][     ][                                  ]`
info chunk ^^^^^^^^^^ chunk2
this is returned

when a user frees up memory, it simply sets the `inuse` flag to false.
however, to avoid fragmentation, unused chunks are merged together.

in this diagram, both chunk and chunk2 are freed.

`[   ][     ][          ][     ][                                  ]`
info chunk chunk2

`[   ][     ][                                                     ]`
info chunk

Note: for searching through usable chunks, I simply added its size to the
original pointer.

`[   ][     ][          ][     ][                                  ]`
info chunk chunk2
^ ^
ptrA ptrB

by adding sizeof(HeapChunk) and chunk->size, we are able to go to the next chunk
without using a linked list. however, this means that the chunks has to be
arranged linearly and without gaps.
