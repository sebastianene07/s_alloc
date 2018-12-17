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

#ifndef __S_HEAP_H
#define __S_HEAP_H

#include <stdint.h>

#include "list.h"

/****************************************************************************
 * Public types
 ****************************************************************************/

/* This structure keeps track of the memory chunk size */

typedef struct {
  uint32_t used : 1;  /* used/unused chunk */
  uint32_t size : 31; /* size of the chunk without header in blocks number */
} mem_mask_t;

/* The memory chunk is represented as a node in a double linked list */

typedef struct mem_node_info_s
{
  mem_mask_t mask;            /* Chunk information as size */
  void *chunk_addr;           /* Start of an allocated chunk */
  struct list_head node_list; /* Next/Prev chunk node */
} mem_node_t;

/* The heap memory structure */

typedef struct {
  struct list_head g_free_heap_list;
  struct list_head g_used_heap_list;

  /* Memory boundaries */

  void *heap_mem_start;
  void *heap_mem_start_unaligned;
  void *heap_memory_end;

  /* Size config */

  size_t block_size;
  size_t num_blocks;
  size_t total_size;
} heap_t;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * s_init() - Initialize heap memory.
 *
 * @start_heap_unaligned: The start of the unaligned memory region for HEAP.
 * @end_heap: The end of the HEAP region.
 *
 * Initialize a heap strucure and create the first free node that will hold
 * all the blocks.
 *
 * Return: No return value.
 */
void s_init(heap_t *my_heap,
            void *start_heap_unaligned,
            void *end_heap,
            size_t block_size);

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
void *s_alloc(size_t len, heap_t *my_heap);

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
void s_free(void *ptr, heap_t *my_heap);

#endif /* __S_HEAP_H */
