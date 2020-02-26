# Filename: src/cmd/Makefile
# Project: isbn-hyphenate
# Brief: Hyphenate ISBNs using the range table from International ISBN Agency
#
# Copyright (C) 2016-2019 Guy Shaw
# Written by Guy Shaw <gshaw@acm.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PROGRAM := isbn-hyphenate
SRCS = $(PROGRAM).c
OBJS = $(PROGRAM).o
LIBS := ../isbn-xml-to-fst/isbn-xml-to-fst.o  ../libfst/libfst.a  ../libcscript/libcscript.a  -lxml2

CC := gcc
CONFIG :=
CFLAGS := -g -Wall -Wextra -fPIC
CPPFLAGS := -I../inc

.PHONY: all test clean-test clean show-targets

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(CONFIG) $(OBJS) $(LIBS)

test: $(PROGRAM)
	@cd test && make test

clean-test:
	cd test && make clean

clean: clean-test
	rm -f $(PROGRAM) core a.out *.o *.a

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
