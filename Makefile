#TARGET := default

OUT = allocator

ifeq ($(TARGET), )
	SRC := $(wildcard *.c)
endif

ifeq ($(TARGET), nrf52840)
	SRC := $(wildcard s_heap*.c)
endif

OBJS := $(patsubst %.c,%.o,$(SRC))
ARCH_FLAGS:=${CFLAGS} -I$(TOPDIR)/include

all: $(OBJS)
ifeq ($(TARGET), )
	gcc -m32 -g $(OBJS)  -o $(OUT)
endif

ifeq ($(TARGET), nrf52840)
	${PREFIX}ar -rc $(TOPDIR)/$(TMP_LIB) $(OBJS)
endif

%.o : %.c
ifeq ($(TARGET), )
	gcc -m32 -c -g $< -o $@
endif

ifeq ($(TARGET), nrf52840)
	${PREFIX}gcc $(ARCH_FLAGS) -c $< -o $@
endif

.PHONY: clean

clean:
	rm -f $(OUT) *.o
