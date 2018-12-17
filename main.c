#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "s_heap.h"

void s_dbg_heap(heap_t *my_heap)
{
  printf("\n################ Heap details ####################\n");
  printf("Start addr: 0x%x\n", my_heap->heap_mem_start);
  printf("End addr: 0x%x\n", my_heap->heap_memory_end);
  printf("block size: %u\n", my_heap->block_size);

  printf("################ Alocated blocks ##################\n");

  mem_node_t *node = NULL;
  list_for_each_entry (node, &my_heap->g_used_heap_list, node_list)
  {
    printf("leaked block start = 0x%x, size = %u blocks\n",
           node->chunk_addr,
           node->mask.size);
  }

  printf("################ Free blocks ##################\n");

  node = NULL;
  list_for_each_entry (node, &my_heap->g_free_heap_list, node_list)
  {
    printf("block start = 0x%x, size = %u blocks\n",
           node->chunk_addr,
           node->mask.size);
  }
}

int main(void)
{
  static heap_t my_heap = {0};
  const size_t sz = 1024 * 1024;
  const size_t block_size = 16;
  const uint32_t TEST_ARRAY_SIZE = 100;

  void *start_addr = malloc(sz);
  assert(start_addr);

  s_init(&my_heap,
         start_addr,
         start_addr + sz,
         block_size);

  uint32_t *ptrs[TEST_ARRAY_SIZE];
  memset(&ptrs[0], 0, TEST_ARRAY_SIZE);

  for (int i = 0; i < TEST_ARRAY_SIZE; i++)
  {
    size_t sz = rand() % 10000;
    ptrs[i] = s_alloc(sz, &my_heap);
    printf("Allocated block size %d addr 0x%x\n", sz, ptrs[i]);
  }

  for (int i = 0; i < TEST_ARRAY_SIZE; i++)
  {
    s_free(ptrs[i], &my_heap);
  }

  s_dbg_heap(&my_heap);

  free(start_addr);
  return 0;
}
