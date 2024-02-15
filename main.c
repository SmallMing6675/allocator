#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>

#define HEAP_SIZE 4096

typedef struct HeapChunk {
  bool inuse;
  uint16_t size;
  uint16_t previous_size;
} HeapChunk;

typedef struct HeapInfo {
  size_t available_size;
} HeapInfo;

static HeapInfo *heap = NULL;
static void *heap_start = NULL;

void *_get_next_block(void *ptr, uint16_t size) {
  void *next = ptr + sizeof(HeapChunk) + size;
  if ((next - (void *)heap) > HEAP_SIZE) {
    return NULL;
  }
  return next;
}
void *_get_prev_block(void *ptr, uint16_t size) {
  return ptr - size - sizeof(HeapChunk);
}

bool _is_last_chunk(HeapChunk *ptr) {
  return _get_next_block(ptr, ptr->size) > (void *)(heap + HEAP_SIZE);
}

HeapChunk *_get_first_block(const void *ptr) {
  return (HeapChunk *)(ptr + sizeof(HeapInfo));
}

HeapInfo *init_heap() {
  heap_start = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (heap_start == MAP_FAILED) {
    fprintf(stderr, "Failed to initialize heap\n");
    exit(EXIT_FAILURE);
  }

  heap = (HeapInfo *)heap_start;
  HeapChunk *first_chunk = heap_start + sizeof(HeapInfo);
  first_chunk->size = HEAP_SIZE - sizeof(HeapInfo) - sizeof(HeapChunk);
  first_chunk->inuse = false;
  first_chunk->previous_size = 0;
  heap->available_size = first_chunk->size;
  return heap;
}

void destroy_heap() {
  if (heap != NULL) {
    munmap(heap_start, HEAP_SIZE);
    heap = NULL;
  }
}

void *heap_alloc(uint16_t size) {
  /*

     old chunk
     [                           ]

     chunk        new_chunk
    [     ][                     ]*/

  size = (size + 15) & ~0xf; /*aligns it to 16 bytes*/
  if (heap->available_size < size) {
    return (void *)-1;
  }
  HeapChunk *chunk = _get_first_block(heap);

  while (chunk->size < size || chunk->inuse) {
    chunk = _get_next_block(chunk, chunk->size);
  }

  if (chunk->size == size) {
    return (void *)chunk + sizeof(HeapChunk);
  }

  HeapChunk *new_chunk = _get_next_block(chunk, size);
  new_chunk->size = chunk->size - size - sizeof(HeapChunk);
  new_chunk->previous_size = size;
  new_chunk->inuse = false;
  chunk->size = size;
  chunk->inuse = true;

  heap->available_size -= size + sizeof(HeapChunk);

  return (void *)chunk + sizeof(HeapChunk);
}

void heap_free(void *ptr) {
  /*    prev_chunk        chunk            next_chunk
     [            ][                 ][                  ]
        */
  HeapChunk *chunk = ptr - sizeof(HeapChunk);

  chunk->inuse = false;
  if (chunk->previous_size) {
    HeapChunk *prev_chunk =
        (void *)chunk - chunk->previous_size - sizeof(HeapChunk);
    if (!prev_chunk->inuse) {
      prev_chunk->size = prev_chunk->size + sizeof(HeapChunk) + chunk->size;
      /*merge those two chunks into a larger chunk*/
    }
    prev_chunk->inuse = false;

    HeapChunk *next_chunk = _get_next_block(prev_chunk, prev_chunk->size);
    if (next_chunk) {
      next_chunk->previous_size = prev_chunk->size;
    }
  }
  HeapChunk *next_chunk = _get_next_block(chunk, chunk->size);
  if (next_chunk) {
    if (!next_chunk->inuse) {
      chunk->size = chunk->size + sizeof(HeapChunk) + next_chunk->size;
    }
  }
  heap->available_size += chunk->size + sizeof(HeapChunk);
}
void print_heap(const HeapInfo *heap_start) {
  const unsigned char *p = (const unsigned char *)heap_start;
  HeapChunk *head = _get_first_block(heap_start);

  while ((void *)head < ((void *)heap + HEAP_SIZE)) {

    printf("Chunk: location relative to heap: %d size: %d previous size: %d "
           "inuse: %b \n",
           (void *)head - (void *)heap, head->size, head->previous_size,
           head->inuse);
    head = _get_next_block(head, head->size);
  }
  printf("\n");
}

struct Example {
  char buffer[32];
};
int main() {
  init_heap();

  struct Example *x = heap_alloc(32);
  struct Example *y = heap_alloc(64);

  printf("%p, %p \n", x, y);
  printf("%d, %d \n", sizeof(*x), sizeof(*y));
  strcpy(x->buffer, "Hello World!");
  strcpy(y->buffer, "example usage");

  HeapChunk *first = _get_first_block(heap);

  heap_free(x);

  print_heap(heap);
  heap_free(y);

  print_heap(heap);
}
