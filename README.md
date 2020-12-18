# Simple Allocator

Simple memory allocator using Linux linked list implementation.

## Getting Started

Make sure you have gcc tools installed on your computer and type:

```
make
```

This will build the static library lib_allocator.a. Build the test for the
allocator:

```
make test
```

This will produce an executable on your host machine which will run 
the functional tests for the allocator.

Example output:

```
bash:~/> ./allocator

Allocated block size 48 addr 0x7f8830005bc0 fill_with:c4 
Re-allocated block size 65 addr 0x7f8830005c20 fill with:c4
Allocated block size 11 addr 0x7f8830005ca0 fill_with:c5 
Allocated block size 52 addr 0x7f8830005ce0 fill_with:c6 
Re-allocated block size 64 addr 0x7f8830005d40 fill with:c6
Allocated block size 76 addr 0x7f8830005da0 fill_with:c7 
Re-allocated block size 37 addr 0x7f8830005e20 fill with:c7
7 is NULL address !
36 is NULL address !
113 is NULL address !

################ Heap details ####################
Start addr: 0x7f8830000000
End addr: 0x7f8830100000
block size: 32
################ Alocated blocks ##################
################ Free blocks ##################
block start = 0x7f8830000020, size = 32767 blocks

##############

```

## Library usage

The library has the following API :

``` 
s_init

/* Initialize the library by specifing the start of the heap
 * address and the end address. Specify a heap structure as
 * an input. This will be used as a container to store specific
 * data for the heap.
 */
```

``` 
s_alloc 

/* As the name suggests this allocates a chunk of memory in a 
 * specified heap. Returns NULL on failure.
```

```
s_realloc 

/* Re-allocate a heap region in a new chunk. This operation is
 * costly because it involves data copy if we want to strecth
 * an existing chunk. Moreover it has to re-order the block
 * that was freed.
```

``` 
s_free 

/* Eliberate a previously allocated block of memory from a specified
 * heap and merge the free blocks in the free list if they are 
 * adjacent.
```

How can we face fragmentation issues ?

You can instantiate multipple heap buckets by calling ``` s_init ``` with the
appropriate parameters. The advantages of having multipple buckets is that
you can reduce fragmentation by keeping large blocks in one heap and small ones
in another heap.

You can easily create a wrapper on top of ``` s_alloc () ``` and
``` s_free () ``` that will select which heap bucket to use based on the
requested memory chunk size.

Are there any limitations ?

The largest size of an allocation should not be greater than :
2 ^ 31 * BLOCK_SIZE where the BLOCK_SIZE is 32 so the largest chunk can
be 2 ^ 36 bytes = 64Gb.
Every chunk of memory has a header where we store the chunk size and this
value can be adjusted by needs. Also the BLOCK_SIZE value can be adjusted
but make sure that you use a value that doesn't waste space if your alocations
are typically small.


Signed-off-by: Sebastian Ene <sebastian.ene07@gmail.com>
