CFLAGS=-g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	$(CC) -o cc $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

test: cc
	./test.sh

clean:
	rm -f cc *.o *~ tmp* core.* a.out

as-show:
	cc -o cc main.c tokenize.c parse.c codegen.c map.c vector.c register.c util.c
	./cc ${op}> tmp.s

.PHONY: test clean
