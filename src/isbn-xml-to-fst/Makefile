
.PHONY: ec eh diff all run run-gdb run-valgrind clean
.PHONY: import-table

SRCS_H := $(wildcard *.h)
SRCS_C := $(wildcard *.c)
SRCS   := $(SRCS_H) $(SRCS_C)
OBJS   := $(patsubst %.c, %.o, $(SRCS_C))

CC := clang
CFLAGS := -ggdb -g3 -Wall -Wextra -fPIC -I../inc -I. -I/usr/include/libxml2

all: $(OBJS)

diff:
	rcs-diff --diff-ok -u $(SRCS)

clean:
	rm -f $(OBJS) core
	rm -rf tmp

isbn-range.xml:
	./import-range-table

tmp:
	mkdir tmp
