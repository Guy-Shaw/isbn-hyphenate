/*
 * Filename: libfst-impl.h
 * Library: libfst
 * Brief: Definitions needed for implementation of FST library
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

#ifndef _LIBFST_IMPL_H
#define _LIBFST_IMPL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <vec.h>
#include <fst-alloc.h>

/*
 * NEW
 * ---
 * NSOV := Next State OR Value
 *
 * typedef struct nsov nsov_t;
 *
 */

#include <stdio.h>
    // Import type FILE
#include <unistd.h>
    // Import type size_t
#include <stdbool.h>
    // Import constant true


/*
 * FST := Finite-State Transducer
 *
 * fst_t is an array of fst_rule_t
 *
 * fst_rule is a structure containing:
 *     number of transitions
 *     array of transitions
 * where transition is { byte -> next_state } pair
 *
 * fst_rule is a structure containing:
 *     number of transitions
 *     array of transitions
 * where transition is { byte -> next_state } pair
 *
 * FST methods
 * ------------
 *   fst_new(void) -> fst_t *new_fst
 *       Create a new, empty FST.
 *
 *   fst_validate(fst *) -> int errcode
 *
 *   fst_add_transition(fst *, state_t state, int chr, size_t val) \
 *           -> int errcode
 *       Add the transition { chr -> val } to the rules at state 'state'.
 *       Add a transition to an existing rule.
 *       If the rule needs to be reallocated, then it may have to be relocated.
 *
 *   fst_rule_lookup(fst *, state_t state, int chr) \
 *           -> state_t next_state_or_value
 *       Given an existing state, lookup the transition for the given 'chr'.
 *       Return (UNDEF_STATE) if there is no trasition for 'chr'.
 *
 *       #define UNDEF_STATE ((state_t)-1)
 *
 *   fst_add_state(fst *) -> state_t state_nr
 *       Grow the FST state array by one element.
 *       The actual underlying implementation may grow capacity
 *       by some arbirary amount to reduce the number of reallocations
 *       and possible relocations.  If there is spare room,
 *       the growth can be accomplished with just a change in the
 *       recorded current size (as opposed to capacity)
 *
 *
 * Auxilliary data may used to keep track of the actual
 * locations of the array of states
 * and the arrays of rules for each state.
 *
 */

// #################### Data types

typedef size_t state_t;
typedef int    err_t;

#define UNDEF_STATE ((state_t)-1)

union next {
    state_t next_state;
    val_t   val;
};

typedef union next next_t;

struct enext {
    int    err;
    next_t nxt;
};

typedef struct enext enext_t;

static inline state_t
as_state(next_t nxt)
{
    return (nxt.next_state);
}

static inline val_t
as_value(next_t nxt)
{
    return (nxt.val);
};

struct trans {
    int    t_chr;
    next_t t_next;
};

typedef struct trans trans_t;

struct fst_state {
    trans_t *transv;
    size_t ntrans;
};

typedef struct fst_state fst_state_t;

#if 0
typedef vec_t fst_t;
#endif

/*
 * A final state has just 1 transition, and that transition has
 * t_chr == 0.  That is, it is structured as if it is a transition
 * from the NUL byte to some new state, but for a final state,
 * what would have been the next state is a value to be returned.
 */

static inline bool
is_final_state(fst_t *fst, state_t state)
{
    fst_state_t *s0;
    fst_state_t *sv;
    size_t ntrans;
    int chr;

    s0 = (fst_state_t *)fst->base;
    sv = s0 + state;
    ntrans = sv->ntrans;
    if (ntrans > 1) {
        return (false);
    }
    chr = sv->transv[0].t_chr & 0xFF;
    return (chr == 0);
}

// #################### Implementation-private Functions

extern err_t fst_validate(fst_t *fst);
extern void fdump_fst(FILE *f, fst_t *fst);
extern enext_t fst_rule_lookup(fst_t *fst, state_t state, int chr);
extern err_t fst_add_transition(fst_t *fst, state_t state, int chr, next_t next);
extern state_t fst_add_state(fst_t *fst);

#ifdef  __cplusplus
}
#endif

#endif  /* _LIBFST_IMPL_H */
