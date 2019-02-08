/*
 * This file is part of the CatOS distribution https://github.com/catos.
 *
 * Copyright (c) 2018 Sebastian Ene.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "s_heap.h"

/**
 * list_size - Get the size of the linked list.
 *
 * @list: The linked list.
 *
 * Return: The size of the linked list or 0.
 */
static size_t list_size(struct list_head *list)
{
  if (list == NULL)
  {
    return 0;
  }

  int i = 0;
  mem_node_t *node = NULL;
  list_for_each_entry (node, list, node_list)
  {
    i++;
  }

  return i;
}

/**
 * list_sort - Sort the list with comparator function as argument.
 *
 * @list: the linked list to sort.
 * @cmp: callback function for comparator.
 *
 * This is an O(N^2) sorting implementation on a Linux style double linked list
 */
static void list_sort(struct list_head *list,
                      comparator_cb cmp)
{
  struct list_head *node_1 = NULL;
  struct list_head *node_2 = NULL;
  bool ok = true;

  if (list_size(list) == 1)
  {
    return;
  }

  while (ok)
  {
    ok = false;
    list_for_each_stop(node_1, list->prev, list)
    {
      node_2 = node_1->next;

      if (cmp(node_1, node_2) > 0)
      {
        struct list_head *tmp1_prev;
        tmp1_prev = node_1->prev;
        tmp1_prev->next = node_2;

        struct list_head *tmp2_next;
        tmp2_next = node_2->next;
        tmp2_next->prev = node_1;

        node_1->prev = node_2;
        node_1->next = tmp2_next;

        node_2->prev = tmp1_prev;
        node_2->next = node_1;

        ok = true;

        if (node_1 == list->prev)
        {
          return;
        }
      }
    }
  }
}

/**
 * s_init() - Initialize heap memory.
 *
 * @start_heap_unaligned: The start of the unaligned memory region for HEAP.
 * @end_heap: The end of the HEAP region.
 * @block_size: The block size.
 *
 * Initialize a heap strucure and create the first free node that will hold
 * all the blocks.
 *
 * Return: No return value.
 */
void s_init(heap_t *my_heap,
            void *start_heap_unaligned,
            void *end_heap,
            size_t block_size)
{
  assert(block_size == sizeof(mem_node_t));
  assert(end_heap > start_heap_unaligned);

  if (my_heap == NULL ||
      start_heap_unaligned == NULL ||
      end_heap == NULL)
    {
      assert(false);
      return;
    }

  /* Save the block size */

  my_heap->block_size = block_size;
  my_heap->heap_memory_end = end_heap;
  my_heap->heap_mem_start_unaligned = start_heap_unaligned;

  INIT_LIST_HEAD(&my_heap->g_free_heap_list);
  INIT_LIST_HEAD(&my_heap->g_used_heap_list);

  /* Align heap_mem_start to HEAP_BLOCK_SIZE */

  my_heap->heap_mem_start = (void *)(((uint32_t)start_heap_unaligned) +
    ((uint32_t)start_heap_unaligned) % block_size);

  /* Count the number of blocks */

  my_heap->num_blocks =
    ((uint32_t)end_heap - (uint32_t)my_heap->heap_mem_start) / block_size;

  memset(my_heap->heap_mem_start, 0, my_heap->num_blocks * block_size);

  /* Add the first node */

  mem_node_t *start_node = (mem_node_t *)my_heap->heap_mem_start;
  start_node->mask = (mem_mask_t) {
    .used = 0,
    .size = my_heap->num_blocks - 1,
  };

  start_node->chunk_addr = my_heap->heap_mem_start + block_size;

  INIT_LIST_HEAD(&start_node->node_list);

  /* Add the init node to the free list */

  list_add(&start_node->node_list, &my_heap->g_free_heap_list);
}

/**
 * s_alloc() - Allocate a memory chunk in a specified heap.
 *
 * @len: The requested memory size.
 * @my_heap: The pool of memory from where we allocate.
 *
 * This function reserves a continious block of memory.
 *
 * Return: A void pointer on success otherwise NULL.
 *
 * On success it returns the address of the new memory block otherwise
 * it returns NULL.
 */
void *s_alloc(size_t len, heap_t *my_heap)
{
  /* Start by looking in the list and search for an empty block with size >= len */

  mem_node_t *node = NULL;
  list_for_each_entry (node , &my_heap->g_free_heap_list, node_list)
  {
    if (node->mask.size * my_heap->block_size >= len)
    {
      /* Compute the address and verify if we are out of bounds */

      size_t blocks = len / my_heap->block_size +
        ((len % my_heap->block_size) ? 1 : 0);

      mem_node_t *free_node = node + blocks + 1;
      if ((void *)free_node + 2 * my_heap->block_size > my_heap->heap_memory_end)
      {
        return NULL;
      }

      /* Remove the node from the free list */

      node->mask.used = 1;

      list_del(&node->node_list);
      list_add(&node->node_list, &my_heap->g_used_heap_list);

      assert(node->mask.size - blocks - 1 > 0);

      free_node->mask.used = 0;
      free_node->mask.size = node->mask.size - blocks - 1;
      free_node->chunk_addr = free_node + 1;

      node->mask.size = blocks;

      list_add(&free_node->node_list, &my_heap->g_free_heap_list);
      return node->chunk_addr;
    }
  }

  return NULL;
}

/**
 * addr_comparator() - Address comparator.
 *
 * @node_1: The first node's address.
 * @node_2: The second's node address.
 *
 * This utility function is used by the sorting algoritm to compare the address
 * of two memory nodes.
 *
 * Return: -1 in case the second node's address is greater or equal to the firs
 * node's address otherwise 1.
 *
 */
static int addr_comparator(struct list_head *node_1, struct list_head *node_2)
{
  mem_node_t *mem_node_1 = list_entry(node_1, mem_node_t, node_list);
  mem_node_t *mem_node_2 = list_entry(node_2, mem_node_t, node_list);

  if (mem_node_1->chunk_addr <= mem_node_2->chunk_addr)
  {
    return -1;
  }
  else
  {
    return 1;
  }
}

/**
 * s_free() - Release an allocated block of memory.
 *
 * @ptr: The specified buffer to be freed.
 * @my_heap: The specified heap where the buffer lives in.
 *
 * Free an allocate dmmeory chunk. In case the block does not exist in the
 * used blocks list we assert.
 *
 * Return: None.
 *
 */
void s_free(void *ptr, heap_t *my_heap)
{
  /* Verify if the address is in the HEAP range */

  if (ptr == NULL)
  {
    return;
  }

  mem_node_t *node = NULL;
  bool found_block = false;
  list_for_each_entry (node, &my_heap->g_used_heap_list, node_list)
  {

    if (node->chunk_addr == ptr)
      {
        /* We can't have an allocated block in this list */

        assert(node->mask.used == 1);
        node->mask.used = 0;

        list_del(&node->node_list);
        list_add(&node->node_list, &my_heap->g_free_heap_list);

        found_block = true;
        break;
      }
  }

  /* The specified input address for this function is invalid.
   * Did we encounter a double free memory corruption ?
   */

  assert(found_block == true);

  /* Do we have continious free memory blocks ? If we have, merge them */

  int jx = 0;
  bool merge_blocks;
  list_sort(&my_heap->g_free_heap_list, addr_comparator);

  do {

      merge_blocks = false;
      jx++;

      list_for_each_entry (node, &my_heap->g_free_heap_list, node_list)
      {
        struct list_head *next_node = node->node_list.next;
        if (next_node == &my_heap->g_free_heap_list)
        {
          break;
        }

        mem_node_t *next_mem_node = list_entry(next_node, mem_node_t, node_list);

        /* Check if blocks are adjacent in memory */

        if ((mem_node_t *)node->chunk_addr + node->mask.size == next_mem_node)
        {
          merge_blocks = true;

          /* The next block is my neighbour. Merge with him */

          node->mask.size += next_mem_node->mask.size + 1;
          list_del(next_node);
        }
      }

  } while(merge_blocks);
}
