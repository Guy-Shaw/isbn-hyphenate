/*
 * Filename: vec.h
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

#ifndef _VEC_H
#define _VEC_H

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * NEW
 * ---
 * NSOV := Next State OR Value
 *
 * typedef struct nsov nsov_t;
 *
 */

#include <unistd.h>
    // Import type size_t

struct vec {
    void * base;    // pointer to element 0
    size_t len;     // How many elements
    size_t size;    // How much space allocated; number of elements
    size_t esize;   // Size of each element (since we do not have templates)
};

typedef struct vec vec_t;


extern void vec_alloc(vec_t *vp, size_t nelem);
extern void vec_grow(vec_t *vp, size_t idx);
extern void vec_make_room(vec_t *vp, size_t idx);

#ifdef  __cplusplus
}
#endif

#endif  /* _VEC_H */
