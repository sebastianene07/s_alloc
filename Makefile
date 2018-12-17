OUT = allocator
CFLAGS += -m32
LDFLAGS += ${CFLAGS}
SRC := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRC))

all: $(OBJS)
	gcc ${LDFLAGS} $(OBJS)  -o $(OUT)

%.o : %.c
	gcc ${CFLAGS} -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OUT) *.o
