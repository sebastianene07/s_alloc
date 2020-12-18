#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "s_heap.h"

#define TEST_HEAP_LENGTH_BYTES    (128 * 1024 * 1024)  
#define TEST_ARRAY_SIZE           (200)
#define RANDOM_ALLOCATION_SIZE    (8 * 1024)
#define RANDOM_REALLOCATION_SIZE  (8 * 1024)


void s_dbg_heap(heap_t *my_heap)
{
  printf("\n################ Heap details ####################\n");
  printf("Start addr: 0x%lx\n", (unsigned long)my_heap->heap_mem_start);
  printf("End addr: 0x%lx\n", (unsigned long)my_heap->heap_memory_end);
  printf("block size: %zu\n", my_heap->block_size);

  printf("################ Alocated blocks ##################\n");

  mem_node_t *node = NULL;
  list_for_each_entry (node, &my_heap->g_used_heap_list, node_list)
  {

    printf("leaked block start = 0x%lx, size = %u blocks\n",
           (unsigned long)node->chunk_addr,
           node->mask.size);
    assert(0);
  }

  printf("################ Free blocks ##################\n");

  node = NULL;
  list_for_each_entry (node, &my_heap->g_free_heap_list, node_list)
  {
    printf("block start = 0x%lx, size = %u blocks\n",
           (unsigned long)node->chunk_addr,
           node->mask.size);
  }
}

int main(void)
{
  static heap_t my_heap;

  void *start_addr = malloc(TEST_HEAP_LENGTH_BYTES);
  assert(start_addr);

  s_init(&my_heap,
         start_addr,
         start_addr + TEST_HEAP_LENGTH_BYTES);

  uint32_t *ptrs[TEST_ARRAY_SIZE];
  uint32_t size[TEST_ARRAY_SIZE];

  memset(&ptrs[0], 0, TEST_ARRAY_SIZE);

  s_dbg_heap(&my_heap);

  uint32_t it = 0;
  while (1)
	{

		for (int i = 0; i < TEST_ARRAY_SIZE; i++)
		{
			size_t sz = rand() % RANDOM_ALLOCATION_SIZE;
			size[i] = sz;
			ptrs[i] = s_alloc(sz, &my_heap);
			if (ptrs[i] == NULL)
			{
				size[i] = 0;
				printf("No more space found for %zu bytes return NULL!\n", sz);
				continue;
			}

			/* Set the i value */

			for (int j = 0; j < sz; j++)
			{
				*((uint8_t *)ptrs[i] + j) = i;
			}

			printf("Allocated block size %zu addr 0x%lx fill_with:%x \n", sz,
             (unsigned long)ptrs[i],
             i);

			/* Try realloc if the sz value is modulo 2 */

			if (sz % 2 == 0)
			{
				size_t new_size = rand() % RANDOM_REALLOCATION_SIZE;
				size[i] = new_size;
				ptrs[i] = s_realloc(ptrs[i], new_size, &my_heap);
				if (ptrs[i] == NULL)
				{
					size[i] = 0;
					printf("No more space found for %zu bytes return NULL!\n", sz);
					continue;
				}

				/* Set the i value */

				for (int j = sz; j < new_size; j++)
				{
					*((uint8_t *)ptrs[i] + j) = i;
				}

				printf("Re-allocated block size %zu addr 0x%lx fill with:%x\n",
               new_size, (unsigned long)ptrs[i], i);
			}
		}

		/* Verify if the blocks are overlapping */

		for (int i = 0; i < TEST_ARRAY_SIZE; i++)
			for (int j = 0; j < TEST_ARRAY_SIZE; j++)
			{
				if (i != j)
				{
					uint8_t *up_limit = ((uint8_t *)ptrs[i]) + size[i];
					uint8_t *low_limit = (uint8_t *)ptrs[i];

					if (ptrs[j] == NULL) continue;
					if (low_limit <= (uint8_t *)ptrs[j] &&
							up_limit >= (uint8_t *)ptrs[j])
					{
						printf("(0x%lx - 0x%lx)\n", (unsigned long)low_limit, (unsigned long)up_limit);
						printf("Oh crap !!!\n Blocks 0x%lx (size %u) and 0x%lx (size %u)are"
									 " overlapping\n", (unsigned long)ptrs[i], size[i],
                   (unsigned long)ptrs[j], size[j]);
						assert(false);
					}
				}
			}

		/* verify if we have the correct contents */

		for (int i = 0; i < TEST_ARRAY_SIZE; i++)
		{
			if (ptrs[i] == NULL)
			{
				printf("%d is NULL address !\n", i);
				continue;
			}

			for (int j = 0; j < size[i]; ++j)
			{
				if (*((uint8_t *)ptrs[i] + j) != i)
				{
					printf("Memory corruption detected\n");
					assert(*((uint8_t *)ptrs[i] + j) == i);
				}
			}
		}

		for (int i = 0; i < TEST_ARRAY_SIZE; i++)
		{
			s_free(ptrs[i], &my_heap);
		}

		s_dbg_heap(&my_heap);
		printf("\r\n##############\r\n");
		printf("\r\n##############\r\n");
		printf("\r\nIteration : %u\n", ++it);
		printf("\r\n##############\r\n");
		printf("\r\n##############\r\n");
//    sleep(1);
	}
  free(start_addr);
  return 0;
}
