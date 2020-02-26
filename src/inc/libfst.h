/*
 * Filename: libfst.h
 * Library: libfst
 * Brief: Interface for the FST (Finite-State Transducer) library
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

#ifndef _LIBFST_H
#define _LIBFST_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <vec.h>

#include <stdio.h>
    // Import type FILE
#include <unistd.h>
    // Import type size_t
#include <stdbool.h>
    // Import constant true


// #################### Data types

typedef size_t val_t;


#ifdef LIBFST_IMPL
typedef vec_t fst_t;
#else
struct fst;
typedef struct fst fst_t;
#endif

// #################### Functions

extern void fst_init(fst_t *fst);
extern fst_t *fst_new(void);
extern int fst_add_string(fst_t *fst, const char *str, val_t val);
extern void fdump_fst(FILE *f, fst_t *fst);
extern fst_t *fst_copy_and_pack(fst_t *src_fst);
extern void   fst_pack(fst_t *dst_fst, fst_t *src_fst);
extern size_t fst_measure(fst_t *fst);
extern int fst_lookup_string(fst_t *fst, const char *str, val_t *val_ret_ref);
extern int fst_lookup_prefix(fst_t *fst, const char *str, val_t *val_ret_ref);

#ifdef  __cplusplus
}
#endif

#endif  /* _LIBFST_H */
