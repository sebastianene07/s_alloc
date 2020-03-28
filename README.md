# S Alloc

Simple memory allocator using Linux linked list implementation.

## Getting Started

Make sure you have gcc tools installed on your computer and type:

```
make
```

The library has a test implemented in main.c to verify allocation and free.

## Library usage

You can instantiate multipple heap buckets by calling ``` s_init ``` with the
appropriate parameters. The advantages of having multipple buckets is that
you can reduce fragmentation by keeping large blocks in one heap and small ones
in another heap.

You can easily create a wrapper on top of ``` s_alloc () ``` and
``` s_free () ``` that will select which heap bucket to use based on the
requested memory chunk size.


Signed-off-by: Sebastian Ene <sebastian.ene07@gmail.com>
