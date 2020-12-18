OUT = allocator
LIBRARY_CFLAGS := $(CFLAGS) -Wall -Werror
TOPDIR ?= .
TMP_LIB ?= lib_salloc.a 
LIBRARY := $(TOPDIR)/$(TMP_LIB)
SRC := s_heap.c list.c
TEST_SRC := main.c
OBJS := $(patsubst %.c,%.o,$(SRC))

all: $(OBJS)
	$(PREFIX)ar -rc $(LIBRARY) $(OBJS)

test:
	$(PREFIX)gcc $(LIBRARY_CFLAGS) $(LIBRARY) $(TEST_SRC) -o $(OUT)

%.o : %.c
	$(PREFIX)gcc $(LIBRARY_CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OUT) *.o $(LIBRARY)
