/*
 * Filename: test-libfst.c
 * Library: libfst
 * Brief: Simple test program -- add some words; then look them up
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

#include <stdio.h>
    // Import fprintf()
    // Import var stderr
#include <stdlib.h>
    // Import exit()

#include <libfst.h>

int
main()
{
    fst_t *fst;
    int rc;
    val_t world_val;
   
    fst = fst_new();
    rc = fst_add_string(fst, "hello", 1);
    fdump_fst(stderr, fst);
    if (rc) {
        fprintf(stderr, "rc = %d\n", rc);
        exit(2);
    }

    rc = fst_add_string(fst, "world", 2);
    fdump_fst(stderr, fst);
    if (rc) {
        fprintf(stderr, "rc = %d\n", rc);
        exit(2);
    }

    rc = fst_lookup_string(fst, "world", &world_val);
    if (rc) {
        fprintf(stderr, "lookup of \"world\" failed.\n");
        exit(1);
    }

    printf("lookup(\"world\") -> %zu\n", world_val);


    rc = fst_lookup_string(fst, "worl", &world_val);
    if (rc) {
        fprintf(stderr, "lookup of \"worl\" failed, as it should.\n");
    }
    else {
        printf("lookup(\"world\") -> %zu\n", world_val);
    }

    rc = fst_lookup_string(fst, "worldly", &world_val);
    if (rc) {
        fprintf(stderr, "lookup of \"worldly\" failed, as it should.\n");
    }
    else {
        printf("lookup(\"worldly\") -> %zu\n", world_val);
    }

    rc = fst_lookup_prefix(fst, "worldly", &world_val);
    if (rc) {
        fprintf(stderr, "lookup of \"worldly\" failed.\n");
    }
    else {
        printf("lookup(\"worldly\") -> %zu\n", world_val);
    }

    exit(0);
}
