#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>

#define HEAP_SIZE 4096

typedef struct {
  size_t avaliable_size;
  void *top;
} HeapInfo;

static HeapInfo *heap = NULL;

void init_heap() {
  void *heap_start = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (heap_start == MAP_FAILED) {
    fprintf(stderr, "Failed to initialize heap\n");
    exit(EXIT_FAILURE);
  }

  heap = heap_start;
  heap->avaliable_size = HEAP_SIZE - sizeof(HeapInfo);
  heap->top = heap_start + sizeof(HeapInfo);
}

void *heap_alloc(uint16_t size) {
  if (size > heap->avaliable_size) {
    return NULL;
  }
  void *top = heap->top;
  heap->top += size;
  heap->avaliable_size -= size;
  return top;
};

void heap_reset() { heap->top = (void *)heap + sizeof(HeapInfo); }

int main() { init_heap(); }
