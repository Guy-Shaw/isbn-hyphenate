
LIBRARY := libfst
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

CONFIG   := -DDEBUG -ggdb -g3 -finstrument-functions
CC       := clang
CPPFLAGS := -I../inc
CFLAGS += -std=c99 -Wall -Wextra -fPIC $(CONFIG)

.PHONY: all clean

all: $(LIBRARY).a

$(LIBRARY).a: $(OBJECTS)
	ar crv $(LIBRARY).a $(OBJECTS)

clean:
	rm -f $(LIBRARY).a $(OBJECTS) *.o
	cscope-clean
