CFLAGS=-g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	$(CC) -o cc $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

test: cc
	./test.sh

clean:
	rm -f cc *.o *~ tmp*

.PHONY: test clean
