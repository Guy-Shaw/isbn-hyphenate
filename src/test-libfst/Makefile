
.PHONY: ec eh em diff all run gdb run-valgrind clean

PROGRAM := test-fst
SRCS_H := $(wildcard *.h)
SRCS_C := $(wildcard *.c)
SRCS   := $(SRCS_H) $(SRCS_C)
OBJS   := $(patsubst %.c, %.o, $(SRCS_C))
LIBS   := ../libfst/libfst.a

CC := clang
CFLAGS := -ggdb -g3 -finstrument-functions -Wall -Wextra -I../inc -I.

all: $(PROGRAM)

em:
	vim Makefile

ec:
	vim $(SRCS_C)

eh:
	vim $(SRCS_H)

diff:
	rcs-diff -u $(SRCS)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(CFLAGS) $(OBJS) $(LIBS)

run: $(PROGRAM)
	if [ ! -e tmp ] ; then mkdir tmp ; fi
	./$(PROGRAM) > tmp/test.out 2>tmp/test.err
	ls -lh tmp/test.*

gdb:
	./mklib/run-gdb ./$(PROGRAM) core

run-valgrind:
	valgrind ./$(PROGRAM) > tmp/test.out 2>tmp/test.err

clean:
	rm -f perf.data perf.out
	rm -rf uftrace*
	rm -rf tmp && mkdir tmp
	rm -f $(PROGRAM) core *.o 
