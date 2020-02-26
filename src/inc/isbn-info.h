/*
 * Filename: src/inc/isbn-info.h
 * Project: isbn-hyphenate
 *
 * Copyright (C) 2016-2020 Guy Shaw
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

#ifndef _ISBN_INFO_H
#define _ISBN_INFO_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
    // Import type uint32_t
#include <unistd.h>
    // Import type size_t

#include <libfst.h>
#include <vec.h>

// ########################### Data structures

/*
 * Information about Prefixes, Agencies, and Rules (Range tables)
 * is kept in two arrays.
 *
 * The Prefix array records the following for each prefix:
 *
 *   1) The prefix, itself
 *      (redundant since we navigate to the prefix entry from the ISBN)
 *   2) The start of a set of Rules (range table entries)
 *      This is an array index, not a pointer.
 *   3) How many rules there are for this prefix.
 *
 * The Rules array records the following information
 * about each range table entry:
 *
 *   1) range length
 *   2) range lower bound
 *
 * The range bounds are inclusive.
 * The XML file produced by @{International ISBN Agency}
 * contains explicit { lower-bound , upper-bound } pairs.
 * Both bounds are inclusive.  This is redundant.
 *
 * While our internal data structures are under construction,
 * we keep both bounds in each range table entry.  But,
 * the final read-only data structures do away with the
 * redundant information.
 *
 * ### Final for of range table entries
 *
 * Only a lower bound need be stored for each range table entry.
 * For any entry in the range table, the lower bound of the next
 * range table entry serves as its upper bound (exclusive).
 * There should be no gap.  But if there are, then any gaps
 * would be filled in by explicit INVALID range table entries.
 * The implicit upper bound for the last entry in a range table
 * is \infinity.
 *
 * rule_nr:.
 *   While reading the XML document and accumulating rules,
 *   this is the index of the next rule to be allocated.
 *   When done reading, it is a count of the number of rules.
 *
 * prefix_nr:
 *   While reading the XML document and accumulating prefixes,
 *   this is the index of the next prefix to be allocated.
 *   When done reading, it is a count of the number of rules.
 */

struct isbn_range {
    size_t   rng_len;
    uint32_t rng_lbound;
    uint32_t rng_ubound;
};

typedef struct isbn_range isbn_range_t;

struct isbn_prefix {
    char *prefix;
    char *agency;
    size_t rule_idx;    // Starting index into array of ranges
    size_t nrules;      // How many rules apply to this prefix
};

typedef struct isbn_prefix isbn_prefix_t;


/*
 * These fields keep track of the current state of parsing the XML document.
 *
 * path:
 *     XML Xpath like path from root of document to current component.
 *
 * depth:
 *     keeps track of current depth in tree-walk.
 *
 * cur_prefix:
 *     The Prefix that applies while walking the tree below.
 *
 * cur_agency:
 *     The agency that applies to all elements below.
 *
 * cur_value:
 *     XXX
 *
 * prefix_vec:
 *     An array of information about all prefixes.
 *     The array grows during parsing.
 *     It is resized as needed.
 *
 * ranges_vec:
 *     An array of information about all range tables.
 *     The array grows during parsing.
 *     It is resized as needed.
 *
 * rule_nr:
 *     Count of how many rules the parser has encountered, so far.
 *     Also, index use to append to |ranges_vec[]|.
 *
 * prefix_nr:
 *     Count of how many prefixes the parer has encountered, so far.
 *     Also, index used to append to |prefix_vec[]|.
 *
 * new_prefix:
 *     Place to build the information about a new prefix.
 *     Each occurrence gets copy-appended to |prefix_vec[]|.
 *
 * fst:
 *     The Finite-State Transducer (FST) that translates variable-length
 *     prefixes to rule numbers.
 *
 * err:
 *     Status of the parser and table builder and of the FST builder.
 *     errno semantics.  That is, 0 == success, non-zero is some errno value.
 *
 */

struct isbn_info {
    char **path;
    size_t depth;
    char *cur_prefix;
    char *cur_agency;
    val_t cur_value;
    vec_t prefix_vec;
    vec_t ranges_vec;
    size_t rule_nr;
    size_t prefix_nr;
    isbn_prefix_t new_prefix;
    fst_t *fst;
    int err;
};

typedef struct isbn_info isbn_info_t;

/*
 * @var{path} is not a pathname or a list of pathnames a la env::PATH.
 * Rather, it is a list of xml path components, representing a stick
 * in the tree representation of the XML file.  That is, it is the
 * path from the root down to the node we are currently visiting.
 */

#define XML_MAX_DEPTH 1024

#define UNDEF_INDEX (size_t)(-1)


#ifdef  __cplusplus
}
#endif

#endif  /* _ISBN_INFO_H */
