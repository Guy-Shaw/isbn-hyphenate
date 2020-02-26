/*
 * Filename: vec.c
 * Brief: General purpose variable length vector
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
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import fprintf()
    // Import var stderr
#include <stdlib.h>
    // Import exit()
#include <unistd.h>
    // Import type size_t

#include <vec.h>
#include <fst-alloc.h>

void
vec_alloc(vec_t *vp, size_t nelem)
{
    if (vp->base != NULL) {
        fprintf(stderr, "vec_alloc: already allocated.\n");
        exit(64);
    }
    if (vp->size != 0) {
        fprintf(stderr, "vec_alloc: already has non-zero element count.\n");
        exit(64);
    }
    vp->base = guard_malloc(nelem * vp->esize);
    vp->size = nelem;
}

void
vec_grow(vec_t *vp, size_t idx)
{
    void *new_base;
    size_t new_size;

    new_size = (vp->size * 16) / 10;
    if (idx > new_size) {
        new_size = (idx * 16) / 10;
    }
    new_base = guard_realloc(vp->base, new_size * vp->esize);
    vp->base = new_base;
    vp->size = new_size;
}

void
vec_make_room(vec_t *vp, size_t idx)
{
    if (vp->base == NULL) {
        vec_alloc(vp, 10);
    }
    else {
        if (idx >= vp->size) {
            vec_grow(vp, idx);
        }
    }
}
