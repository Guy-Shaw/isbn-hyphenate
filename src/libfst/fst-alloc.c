/*
 * Filename: fst-alloc.c
 * Brief: Handle allocations for FST data type
 *
 * Copyright (C) 2015-2016 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fst-alloc.h>

#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import fflush()
    // Import fprintf()
    // Import var stderr
#include <stdlib.h>
    // Import exit()
    // Import malloc()
    // Import realloc()
#include <unistd.h>
    // Import type size_t


void
out_of_memory(void)
{
    fprintf(stderr, "Out of memory.\n");
    fflush(stderr);
    exit(64);
}

void *
guard_malloc(size_t sz)
{
    void *ptr;
   
    ptr = malloc(sz);
    if (ptr == NULL) {
        out_of_memory();
    }
    return (ptr);
}

void *
guard_realloc(void *ptr, size_t sz)
{
    void *new_ptr;

    if (ptr == NULL) {
        new_ptr = malloc(sz);
    }
    else {
        new_ptr = realloc(ptr, sz);
    }
    if (new_ptr == NULL) {
        out_of_memory();
    }
    return (new_ptr);
}
