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

#define MAX_LIST_LENGTH_BITS 20
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include "s_heap.h"

/**
 * size_comparator() - Address comparator.
 *
 * @node_1: The first node's address.
 * @node_2: The second's node address.
 *
 * This utility function is used by allocator function to select the node
 * that occupies less blocks
 *
 * Return: -1 if the 2nd node's size is greater than the 1st node's size
 *          1 if the 1st node's size is greater than the 2nd node's size
 *          0 otherwise
 *
 */
static int size_comparator(void *priv,
                           struct list_head *node_1,
                           struct list_head *node_2)
{
  mem_node_t *mem_node_1 = list_entry(node_1, mem_node_t, node_list);
  mem_node_t *mem_node_2 = list_entry(node_2, mem_node_t, node_list);

 return (mem_node_1->mask.size > mem_node_2->mask.size) -
        (mem_node_1->mask.size < mem_node_2->mask.size);
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
            void *end_heap)
{
  size_t block_size = sizeof(mem_node_t);
  mem_node_t *start_node = NULL;

  if (my_heap == NULL ||
      start_heap_unaligned == NULL ||
      end_heap == NULL ||
      end_heap < start_heap_unaligned)
    {
      assert(false);
      return;
    }

  /* Save the block size */

  my_heap->block_size               = block_size;
  my_heap->heap_memory_end          = end_heap;
  my_heap->heap_mem_start_unaligned = start_heap_unaligned;

  INIT_LIST_HEAD(&my_heap->g_free_heap_list);
  INIT_LIST_HEAD(&my_heap->g_used_heap_list);

  /* Align heap_mem_start to HEAP_BLOCK_SIZE */

  my_heap->heap_mem_start = (void *)(((uintptr_t)start_heap_unaligned) +
    ((uintptr_t)start_heap_unaligned) % block_size);

  /* Count the number of blocks */

  my_heap->num_blocks =
    ((uintptr_t)end_heap - (uintptr_t)my_heap->heap_mem_start) / block_size;

  memset(my_heap->heap_mem_start, 0, my_heap->num_blocks * block_size);

  /* Add the first node */

  start_node = (mem_node_t *)my_heap->heap_mem_start;
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

  list_sort(NULL, &my_heap->g_free_heap_list, size_comparator);

  list_for_each_entry (node , &my_heap->g_free_heap_list, node_list)
  {
    /* If the node is used (1) but is in this list assert */

    assert(node->mask.used == 0);

    if ((node->mask.size > 1) &&
        (node->mask.size - 2) * my_heap->block_size >= len)
    {
      /* Compute the address and verify if we are out of bounds */

      size_t blocks = len / my_heap->block_size + 1;

      /* Remove the node from the free list */

      assert(node->mask.used == 0);
      node->mask.used = 1;

      list_del(&node->node_list);
      list_add(&node->node_list, &my_heap->g_used_heap_list);

      assert(node->mask.size - blocks - 1 >= 0);

      int prev_size = node->mask.size;
      node->mask.size = blocks;

      /* Verify if we have free space after this allocated block. */

      /* next_used_node can be == node */

      mem_node_t *free_node = node + blocks + 1;
      if ((void *)free_node + my_heap->block_size > my_heap->heap_memory_end)
      {
        return NULL;
      }

      int new_free_node_size = prev_size - blocks - 1;
      if (new_free_node_size > 0)
      {
        free_node->mask.size = new_free_node_size;
        free_node->mask.used = 0;
        free_node->chunk_addr = free_node + 1;
        list_add(&free_node->node_list, &my_heap->g_free_heap_list);
      }
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
 * Return: -1 if the 2nd node's address is greater than the 1st node's address
 *          1 if the 1st node's address is greater than the 2nd node's address
 *          0 otherwise

 *
 */
static int addr_comparator(void *priv,
                           struct list_head *node_1,
                           struct list_head *node_2)
{
  mem_node_t *mem_node_1 = list_entry(node_1, mem_node_t, node_list);
  mem_node_t *mem_node_2 = list_entry(node_2, mem_node_t, node_list);

  return (mem_node_1->chunk_addr > mem_node_2->chunk_addr) -
         (mem_node_1->chunk_addr < mem_node_2->chunk_addr);
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
    /* We can't have a free block in this list */

    assert(node->mask.used == 1);

    if (node->chunk_addr == ptr)
      {
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

  bool merge_blocks;
  list_sort(NULL, &my_heap->g_free_heap_list, addr_comparator);

  do {

      merge_blocks = false;

      list_for_each_entry (node, &my_heap->g_free_heap_list, node_list)
      {
        struct list_head *next_node = node->node_list.next;
        if (next_node == &my_heap->g_free_heap_list)
        {
          /* We reached end */

          break;
        }

        mem_node_t *next_free_node = list_entry(next_node, mem_node_t, node_list);

        /* We can't have an used block in the free heap list */

        assert(next_free_node->mask.used == 0);

        for (int offset = 0; offset < 2; offset++)
        {
          mem_node_t *next_possible_node = (mem_node_t *)node->chunk_addr + node->mask.size + offset;
          if (next_possible_node->chunk_addr == ((void *)next_possible_node + sizeof(mem_node_t)))
          {
            /* We almost sure have a new node !!! or we may be totally wrong */

            /* Verify if the size makes sense */

            if (next_possible_node->mask.used == 1 ||
                next_possible_node->mask.size == 0 ||
                next_possible_node->mask.size > 0xFFFF)
            {
              continue;
            }

            /* Check if blocks are adjacent in memory */

            if (next_possible_node == next_free_node)
            {
              merge_blocks = true;

              /* The next block is my neighbour. Merge with him */

              assert(node->mask.used == 0);

              node->mask.size += next_free_node->mask.size + 1;
              list_del(next_node);
              break;
            }
          }
        }
      }

  } while(merge_blocks);
}

/**
 * s_realloc() - Re-allocate a memory block with a new specified size.
 *
 * @ptr: Previously allocated buffer with s_alloc or NULL in case this is a new
 *       allocation.
 * @size: The size of the new alocation or 0 if we want to free ptr memory.
 * @my_heap: The specified heap where the buffer lives in.
 *
 * Resize a block of memory. In case the block does not exist in the
 * used blocks list we assert.
 *
 * Return: None.
 *
 */
void *s_realloc(void *ptr, size_t size, heap_t *my_heap)
{
  if (ptr == NULL)
  {
    return s_alloc(size, my_heap);
  }

  if (size == 0)
  {
    s_free(ptr, my_heap);
    return NULL;
  }

  mem_node_t *node = NULL;
  bool found_block = false;
  list_for_each_entry (node, &my_heap->g_used_heap_list, node_list)
  {
    /* We can't have a free block in this list */

    assert(node->mask.used == 1);

    if (node->chunk_addr == ptr)
      {
        found_block = true;
        break;
      }
  }

  /* The specified input address for this function is invalid.
   * Did we encounter a double free memory corruption ?
   */

  assert(found_block == true);

  uint8_t *new_buffer = s_alloc(size, my_heap);
  if (new_buffer == NULL)
  {
    /* Free should be done by caller */
    return NULL;
  }

  /* If we shrink space we need to copy at least these bytes */

  size_t alloc_size = node->mask.size * my_heap->block_size;
  size_t min_copy_size = size > alloc_size ? alloc_size :
    size;
  memcpy(new_buffer, ptr, min_copy_size);
  s_free(ptr, my_heap);
  return new_buffer;
}
