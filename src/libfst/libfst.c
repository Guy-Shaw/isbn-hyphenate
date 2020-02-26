/*
 * Filename: libfst.c
 * Brief: Finite-State Transducer (FST) library
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

#include <errno.h>
    // Import var EDOM
    // Import var EEXIST
    // Import var ELOOP
    // Import var ENOENT
#include <stdbool.h>
    // Import type bool
    // Import constant false
    // Import constant true
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import type FILE
    // Import fprintf()
    // Import var stderr
#include <stdlib.h>
    // Import exit()
#include <string.h>
    // Import memcpy()
#include <unistd.h>
    // Import type size_t

#define LIBFST_IMPL
#include <libfst.h>
#include <libfst-impl.h>

/*
 * Do not start off with 0 states.
 * Instead, start with 1 state with no transitions.
 * This is because state 0 is special.
 * Transitions to state 0 are final states.
 *
 * XXX Maybe we should use some other value to denote a final state.
 * XXX For what we are doing now, it does not matter, because
 * XXX our FST does not do loops.
 */

void
fst_init(vec_t *vp)
{
    fst_state_t *s0;

    vec_make_room(vp, 10);
    vp->len = 0;
    s0 = (fst_state_t *) vp->base;
    s0->transv = NULL;
    s0->ntrans = 0;
}

vec_t *
vec_new(size_t esize)
{
    vec_t * new_vec;
    new_vec = guard_malloc(sizeof(vec_t));
    new_vec->base = NULL;
    new_vec->len  = 0;
    new_vec->size = 0;
    new_vec->esize = esize;
    return (new_vec);
}

vec_t *
fst_new()
{
    vec_t *vp;
    vp = vec_new(sizeof (fst_state_t));
    fst_init(vp);
    return (vp);
}

err_t
fst_validate(fst_t *fst)
{
    fst_state_t *s0;
    fst_state_t *sv;
    size_t ntrans;
    size_t trnr;
    state_t state;

    if (fst == NULL) {
        fprintf(stderr, "fst==NULL\n");
        exit(32);
    }
    if (fst->base == NULL) {
        fprintf(stderr, "fst->base == NULL\n");
        exit(32);
    }
    s0 = (fst_state_t *)fst->base;
    for (state = 0; state < fst->len; ++state) {
        sv = s0 + state;
        ntrans = sv->ntrans;
        for (trnr = 0; trnr < ntrans; ++trnr) {
            int chr;
            chr = sv->transv[trnr].t_chr & 0xFF;
            if (chr == '\0') {
                // Validate final tansition
            }
            else {
                // Validate next-character transition
                // Verify that next state is in 0 .. nstates
            }
        }
    }

    return (0);
}

/*
 * Measure the size in bytes of a given FST data structure.
 * This is useful for converting an FST to a single-allocation data structure.
 *
 * An FST that was built incrementally, using fst_add_string()
 * and related functions, consists of many small objects that were
 * each allocated separately.
 *
 * fst_measure() tells how big of a single memory allocation
 * will hold the entire FST, including any padding or overhead.
 *
 */

size_t
fst_measure(fst_t *fst)
{
    fst_state_t *s0;
    fst_state_t *sv;
    size_t ntrans;
    state_t state;
    size_t total_fst_size;

    if (fst == NULL) {
        fprintf(stderr, "fst==NULL\n");
        exit(32);
    }
    if (fst->base == NULL) {
        fprintf(stderr, "fst->base == NULL\n");
        exit(32);
    }

    total_fst_size = 0;
    s0 = (fst_state_t *)fst->base;
    for (state = 0; state < fst->len; ++state) {
        // XXX debug fprintf(f, "State %zu:\n", state);
        total_fst_size += sizeof (fst_state_t);
        sv = s0 + state;
        ntrans = sv->ntrans;
        total_fst_size += ntrans * sizeof (trans_t);
    }
    return (total_fst_size);
}

/*
 * dst_fst is a region of memory that has been allocated to hold
 * a copy of src_fst.
 * The contents of dst_fst is unknown.  It gets written over.
 */

void
fst_pack(fst_t *dst_fst, fst_t *src_fst) {
    char *dst_ptr;
    vec_t *vp;
    fst_state_t *s0;
    fst_state_t *sv;
    size_t nstates;
    size_t ntrans;
    state_t state;
    size_t tsize;

    if (src_fst == NULL) {
        fprintf(stderr, "src_fst==NULL\n");
        exit(32);
    }
    if (src_fst->base == NULL) {
        fprintf(stderr, "src_fst->base == NULL\n");
        exit(32);
    }
    if (dst_fst == NULL) {
        fprintf(stderr, "dst_fst==NULL\n");
        exit(32);
    }

    dst_ptr = (char *)dst_fst;
    vp = (vec_t *)dst_ptr;
    s0 = (fst_state_t *)src_fst->base;
    memcpy((void *)dst_ptr, src_fst, sizeof (fst_t));
    dst_ptr += sizeof (fst_t);
    vp->base = (void *)dst_ptr;
    // After packing, capacity is the same as current size
    vp->size = vp->len;
    
    // Copy state headers.
    // Fill in new .transv pointers
    // after all fst_state_t header have been copied,
    // because then we know where the transition array for state 0 lands.
    // Since .transv is a pointer, they all must be set to the new address.

    fst_state_t *dst_s0;
    fst_state_t *dst_sv;
    dst_s0 = (fst_state_t *)dst_ptr;
    nstates = src_fst->len;
    tsize = nstates * sizeof (fst_state_t);
    memcpy((void *)dst_ptr, (void *)s0, tsize);
    dst_ptr += tsize;

    // Lay down all the transition arrays for all states,
    // and while we are at it, fill in the correct new addresses,
    // in each fst_state header to point to the new .transv location.

    for (state = 0; state < nstates; ++state) {
        dst_sv = dst_s0 + state;
        dst_sv->transv = (trans_t *)dst_ptr;
        sv = s0 + state;
        ntrans = sv->ntrans;
        tsize = ntrans * sizeof (trans_t);
        memcpy((void *)dst_ptr, (void *)sv->transv, tsize);
        dst_ptr += tsize;
    }
}

fst_t *
fst_copy_and_pack(fst_t *src_fst)
{
    fst_t *dst_fst;
    size_t src_fst_size;

    src_fst_size = fst_measure(src_fst);
    dst_fst = (fst_t *)guard_malloc(src_fst_size);
    // XXX append guard pattern to end of dst_fst
    fst_pack(dst_fst, src_fst);
    return (dst_fst);
}

void
fdump_fst(FILE *f, fst_t *fst)
{
    fst_state_t *s0;
    fst_state_t *sv;
    size_t ntrans;
    size_t trnr;
    state_t state;

    if (fst == NULL) {
        fprintf(stderr, "fst==NULL\n");
        exit(32);
    }
    if (fst->base == NULL) {
        fprintf(stderr, "fst->base == NULL\n");
        exit(32);
    }
    s0 = (fst_state_t *)fst->base;
    for (state = 0; state < fst->len; ++state) {
        fprintf(f, "State %zu:\n", state);
        sv = s0 + state;
        ntrans = sv->ntrans;
        for (trnr = 0; trnr < ntrans; ++trnr) {
            int chr;
            chr = sv->transv[trnr].t_chr & 0xFF;
            if (chr == '\0') {
                fprintf(f, "            -> value=%zu\n",
                    as_value(sv->transv[trnr].t_next));
            }
            else {
                fprintf(f, "    %c (%3u) -> %zu\n",
                    chr, chr, as_state(sv->transv[trnr].t_next));
            }
        }
    }
}

/*
 * Given an existing state, lookup the transition for the given 'chr'.
 * Return (UNDEF_STATE) if there is no transition for 'chr'.
 */

enext_t
fst_rule_lookup(fst_t *fst, state_t state, int chr)
{
    fst_state_t *s0;
    fst_state_t *sv;
    enext_t enext;
    size_t i;

    if (state >= fst->len + 1) {
        fprintf(stderr, "%s: invalid state:\n", __FUNCTION__);
        fprintf(stderr, "   state=%zu, nstates=%zu\n", state, fst->len);
        enext.err = EDOM;
        enext.nxt = (next_t)UNDEF_STATE;
        return (enext);
    }

    s0 = (fst_state_t *)fst->base;
    sv = s0 + state;

    // Linear search.
    // Do not assume the transitions are sorted.
    // For a finalized FST, transitions could be sorted,
    // and we would do binary search, or something better.
    for (i = 0; i < sv->ntrans; ++i) {
        if (sv->transv[i].t_chr == chr) {
            enext.err = 0;
            enext.nxt = sv->transv[i].t_next;
            return (enext);
        }
    }
    enext.err = ENOENT;
    enext.nxt = (next_t)UNDEF_STATE;
    return (enext);
}

/*
 * Add the transition { chr -> next } to the rules at state 'state'.
 * Add a transition to an existing rule.
 *
 * The value, 'next' can be either a next state or a vlue to be returned,
 * in the case of a 'final' state.
 *
 * A trasnition from '\0' is a final state.
 * All other transitions are to a next state.
 *
 * Implementation notes
 * --------------------
 * If the rule needs to be reallocated, then it may have to be relocated.
 */

err_t
fst_add_transition(fst_t *fst, state_t state, int chr, next_t next)
{
    fst_state_t *s0;
    fst_state_t *sv;
    size_t ntrans;

    if (state >= fst->len + 1) {
        fprintf(stderr, "%s: invalid state:\n", __FUNCTION__);
        fprintf(stderr, "   state=%zu, nstates=%zu\n", state, fst->len);
        return (EDOM);
    }

    if (chr != 0) {
        state_t new_state;
        new_state = as_state(next);
        if (new_state <= state) {
            fprintf(stderr, "Cycles are not allowed.\n");
            fprintf(stderr, "  %zu -> %zu\n", state, new_state);
            return (ELOOP);
        }
    }

    s0 = (fst_state_t *)fst->base;
    sv = s0 + state;
    ntrans = sv->ntrans;
    size_t new_size = (ntrans + 1) * sizeof (trans_t);
    sv->transv = guard_realloc(sv->transv, new_size);
    sv->transv[ntrans].t_chr  = chr;
    sv->transv[ntrans].t_next = next;
    ++sv->ntrans;
    return (0);
}

/*
 * Grow the FST state array by one element.
 *
 * Implementation notes
 * --------------------
 * The actual underlying implementation may grow capacity
 * by some arbitrary amount to reduce the number of reallocations
 * and possible relocations.  If there is spare room,
 * the growth can be accomplished with just a change in the
 * recorded current size (as opposed to capacity)
 */

state_t
fst_add_state(fst_t *fst)
{
    fst_state_t *s0;
    fst_state_t *sv;
    state_t new_state;

    new_state = fst->len + 1;
    vec_make_room(fst, new_state);
    ++fst->len;
    s0 = (fst_state_t *)fst->base;
    sv = s0 + new_state;
    sv->transv = NULL;
    sv->ntrans = 0;
    return (new_state);
}

/*
 * Add a string to a Finite State Transducer (FST)
 *
 * Given an FST that has already been (partially) built,
 * expand the FST by inserting the transitions that would
 * recognize the new given string.
 *
 * Return an error code if the new string is a duplicate.
 *
 */

int
fst_add_string(fst_t *fst, const char *str, val_t val)
{
    const char *s;
    int chr;
    enext_t enext;
    next_t nxt;
    state_t state;
    state_t new_state;
    int err = 0;

    fst_validate(fst);
    s = str;
    state = 0;
    while (true) {
        chr = *s;
        enext = fst_rule_lookup(fst, state, chr);
        err = enext.err;
        if (err != 0 && err != ENOENT) {
            break;
        }
        nxt = enext.nxt;
        new_state = as_state(nxt);
        if (chr == '\0') {
            if (err == ENOENT) {
                // This is a _final_ state.
                // Store the value (entry number) instead of next state.
                err = fst_add_transition(fst, state, 0, (next_t)val);
                if (err) {
                    fprintf(stderr, "fst_add_transition: err=%d\n", err);
                }
            }
            else {
                // This entire string is a duplicate.
                err = EEXIST;
            }
            // No more characters in this string.  We are done.
            break;
        }
        else if (err == ENOENT) {
            // No match for this character in this state.
            // Add a new state.  Then, add the transition
            // from { current state, chr } -> new state.
            new_state = fst_add_state(fst);
            nxt.next_state = new_state;
            err = fst_add_transition(fst, state, chr, nxt);
            if (err) {
                fprintf(stderr, "fst_add_transition: err=%d\n", err);
            }
        }
        state = new_state;
        ++s;
    }
    return (err);
}

int
fst_lookup(fst_t *fst, const char *str, val_t *ret_val_ref, bool pfx)
{
    const char *s;
    int chr;
    enext_t enext;
    next_t nxt;
    state_t state;
    state_t new_state;
    int err = 0;

    fst_validate(fst);
    s = str;
    state = 0;
    while (true) {
        chr = *s;
        enext = fst_rule_lookup(fst, state, chr);
        err = enext.err;
        if (err != 0 && err != ENOENT) {
            break;
        }
        nxt = enext.nxt;
        new_state = as_state(nxt);
        if (chr == '\0') {
            if (err == ENOENT) {
                return (err);
            }
            else {
                // This entire string is a match.
                // Set the value of the final state
                *ret_val_ref = nxt.val;
                return (0);
            }
            // No more characters in this string.  We are done.
            return (ENOENT);
        }
        else if (pfx && is_final_state(fst, state)) {
            fst_state_t *s0 = (fst_state_t *)fst->base;
            fst_state_t *sv = s0 + state;
            *ret_val_ref = as_value(sv->transv[0].t_next);
            return (0);
        }
        else if (err == ENOENT) {
            return (ENOENT);
        }
        else if (nxt.next_state == 0) {
            *ret_val_ref = nxt.val;
            return (0);
        }

        state = new_state;
        ++s;
    }
    return (err);
}

/*
 * Lookup a string.
 * The entire string must match; otherwise ENOENT is returned.
 */
int
fst_lookup_string(fst_t *fst, const char *str, val_t *ret_val_ref)
{
    return (fst_lookup(fst, str, ret_val_ref, false));
}

/*
 * Lookup a prefix.
 * The entire string can match, but if the FST gets to a final state,
 * then there is no error, and value corresponding to that final state
 * is returned.
 *
 * So, variable-length prefixes can work properly.
 *
 */

int
fst_lookup_prefix(fst_t *fst, const char *str, val_t *ret_val_ref)
{
    return (fst_lookup(fst, str, ret_val_ref, true));
}
