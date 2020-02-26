/*
 * Filename: src/isbn-xml-to-fst/isbn-xml-to-fst.c
 * Project: isbn-hyphenate
 * Library: isbn-xml-to-fst
 * Brief: Parse an ISBN XML file and build a Finite State Transducer
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

#include <ctype.h>
    // Import isdigit()
#include <stdbool.h>
    // Import type bool
    // Import constant false
#include <stddef.h>
    // Import constant NULL
#include <stdint.h>
    // Import type uint64_t
    // Import type uint32_t
#include <stdio.h>
    // Import type FILE
    // Import fprintf()
    // Import fputc()
    // Import fputs()
    // Import printf()
    // Import var stderr
    // Import var stdout
#include <stdlib.h>
    // Import calloc()
    // Import exit()
    // Import free()
    // Import strtoul()
    // Import strtoull()
#include <string.h>
    // Import strcmp()
    // Import strdup()
#include <unistd.h>
    // Import type size_t

#include <errno.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <cscript.h>
#include <fprint.h>
#include <isbn-info.h>
#include <libfst.h>
#include <vec.h>

extern char *program_path;
extern char *program_name;

extern FILE *eprint_fh;
extern FILE *dprint_fh;

extern bool debug;
extern bool verbose;

isbn_info_t *
new_isbn_info(void)
{
    size_t tsize = sizeof (isbn_info_t) + XML_MAX_DEPTH * sizeof (char *);
    isbn_info_t *isbn = (isbn_info_t *)guard_malloc(tsize);
    memset((void *)isbn, 0, sizeof (isbn_info_t));
    isbn->path = (char **)isbn + sizeof (isbn_info_t);
    isbn->prefix_vec.esize = sizeof (isbn_prefix_t);
    isbn->ranges_vec.esize = sizeof (isbn_range_t);
    isbn->fst = fst_new();
    return (isbn);
}

// #################### Path functions

void
fdump_prefix_table(FILE *f, vec_t *prefix_vp)
{
    isbn_prefix_t *pfxtbl;
    size_t n;
    size_t i;

    pfxtbl = prefix_vp->base;
    n = prefix_vp->len;
    fprintl(f, "");
    fprintl(f,"@section prefix-table");
    fprintf(f, "Prefix table: %zu entries.", n);
    fprintl(f, "");
    for (i = 0; i < n; ++i) {
        fprintf(f, "[%3zu] pfx=[%s], agency=[%s], rules={%zu,%zu}",
                i,
                pfxtbl[i].prefix,
                pfxtbl[i].agency,
                pfxtbl[i].rule_idx,
                pfxtbl[i].nrules);
        fprintl(f, "");
    }
    fprintl(f, "@end prefix-table");
}


void
fdump_ranges(FILE *f, vec_t *ranges_vp)
{
    isbn_range_t *ranges;
    size_t n;
    size_t i;

    ranges = ranges_vp->base;
    n = ranges_vp->len;
    fprintl(f, "");
    fprintl(f,"@section ranges");
    fprintl(f, "Ranges");
    fprintl(f, "------");
    for (i = 0; i < n; ++i) {
        fprintf(f, "[%3zu] len=%zu, lbound=%u, ubound=%u",
                i, ranges[i].rng_len, ranges[i].rng_lbound, ranges[i].rng_ubound);
        fprintl(f, "");
    }
    fprintl(f, "@end ranges");
}

void
push_path(isbn_info_t *isbn, char *name)
{
    if (isbn->depth >= XML_MAX_DEPTH) {
        eprintf("Max XML path depth exceeded.  depth=%zu", isbn->depth);
        eprintl("");
        exit(64);
    }
    isbn->path[isbn->depth] = name;
    ++isbn->depth;
}

void
pop_path(isbn_info_t *isbn)
{
    --isbn->depth;
}

void
fprint_path(FILE *f, isbn_info_t *isbn)
{
    size_t depth = isbn->depth;
    size_t i;

    for (i = 0; i < depth; ++i) {
        if (i > 0) {
            fprint(f, "/");
        }
        fprint(f, isbn->path[i]);
    }
    fprintl(f, "");
}

// #################### Functions to build Rules data structures

int
add_prefix(isbn_info_t *isbn, char *pfx, char *agency)
{
    isbn_prefix_t *pfxtbl;
    char numeric_prefix[16];
    size_t rlen;
    size_t i;
    val_t prefix_val;
    int rc;

    vec_make_room(&isbn->prefix_vec, isbn->prefix_nr);
    pfxtbl = isbn->prefix_vec.base;

    rlen = 0;
    for (i = 0; pfx[i]; ++i) {
        if (isdigit(pfx[i])) {
            numeric_prefix[rlen] = pfx[i];
            ++rlen;
        }
        else if (pfx[i] != '-') {
            eprintl("Non-numeric character in prefix.");
            return (2);
        }
    }
    numeric_prefix[rlen] = '\0';

    isbn->new_prefix.prefix = strdup(numeric_prefix);
    isbn->new_prefix.agency = strdup(agency);  // XXX Use str_intern()

    // These fields have been set by add_rule()
    //     .rule_idx
    //     .nrules

    pfxtbl[isbn->prefix_nr] = isbn->new_prefix;

    isbn->new_prefix.rule_idx = UNDEF_INDEX;
    isbn->new_prefix.nrules = 0;

    prefix_val = isbn->cur_value;
    rc = fst_add_string(isbn->fst, numeric_prefix, prefix_val);
    ++isbn->cur_value;
    ++isbn->prefix_nr;
    ++isbn->prefix_vec.len;
    return (rc);
}

int
add_rule(isbn_info_t *isbn, size_t len, uint32_t lbound, uint32_t ubound)
{
    isbn_range_t *ranges;

    vec_make_room(&isbn->ranges_vec, isbn->rule_nr);
    ranges = isbn->ranges_vec.base;
    ranges[isbn->rule_nr].rng_len     = len;
    ranges[isbn->rule_nr].rng_lbound  = lbound;
    ranges[isbn->rule_nr].rng_ubound  = ubound;

    // Update accounting for rules in the current prefix table entry.

    if (isbn->new_prefix.rule_idx == UNDEF_INDEX) {
        isbn->new_prefix.rule_idx = isbn->rule_nr;
        isbn->new_prefix.nrules = 0;
    }

    ++isbn->new_prefix.nrules;
    ++isbn->rule_nr;
    ++isbn->ranges_vec.len;

    return (0);
}

int
add_rule_str(isbn_info_t *isbn, xmlChar *range_str, xmlChar *range_len)
{
    char lo_str[16];
    char hi_str[16];
    uint32_t lo;
    uint32_t hi;
    size_t len;

    size_t rlen;
    size_t i;

    if (range_str == NULL) {
        eprint("No range for ");
        fprint_path(eprint_fh, isbn);
        eprintl("");
        return (2);
    }

    if (range_len == NULL) {
        eprint("No range lengh for ");
        fprint_path(eprint_fh, isbn);
        eprintl("");
        return (2);
    }

    rlen = 0;
    i = 0;
    while (range_str[i] && range_str[i] != '-') {
        if (rlen > 13) {
            return (2);
        }
        lo_str[rlen] = range_str[i];
        ++rlen;
        ++i;
    }
    lo_str[rlen] = '\0';
    if (range_str[i] == '-') {
        ++i;
    }

    rlen = 0;
    while (range_str[i]) {
        if (rlen > 13) {
            return (2);
        }
        hi_str[rlen] = range_str[i];
        ++rlen;
        ++i;
    }
    hi_str[rlen] = '\0';
    len = strtoul((char *)range_len, NULL, 10);
    lo = strtoull(lo_str, NULL, 10);  // XXX just 32 bits
    hi = strtoull(hi_str, NULL, 10);  // XXX just 32 bits
    return (add_rule(isbn, len, lo, hi));
}

// #################### Parsing functions

void
parseRule(isbn_info_t *isbn, xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *range_key;
    xmlChar *length_key;
    int rv;

    push_path(isbn, "Rule");
    cur = cur->xmlChildrenNode;
    range_key = NULL;
    length_key = NULL;
    rv = 0;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"Range"))) {
            range_key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (verbose) {
                fprint_path(vprint_fh, isbn);
                vprintl_kv("Range", (char *)range_key);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"Length"))) {
            length_key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            vprintl_kv("Length", (char *)length_key);
        }
        cur = cur->next;
    }

    rv = add_rule_str(isbn, range_key, length_key);

    if (range_key != NULL) {
        xmlFree(range_key);
    }

    if (length_key != NULL) {
        xmlFree(length_key);
    }

    pop_path(isbn);

    if (rv) {
        eprintl("Error in parseRule().");
        exit(2);
    }
}

void
parseRules(isbn_info_t *isbn, xmlDocPtr doc, xmlNodePtr cur)
{
    push_path(isbn, "Rules");
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"Rule"))) {
            parseRule(isbn, doc, cur);
        }
        cur = cur->next;
    }
    pop_path(isbn);
    return;
}

int
parseGroup(isbn_info_t *isbn, xmlDocPtr doc, xmlNodePtr cur)
{
    int group_err = 0;

    push_path(isbn, "Group");
    cur = cur->xmlChildrenNode;

    if (isbn->cur_prefix != NULL) {
        free(isbn->cur_prefix);
        isbn->cur_prefix = NULL;
    }

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"Prefix"))) {
            xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            isbn->cur_prefix = strdup((char *)key);
            vprintl_kv("Prefix", (char *)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"Agency"))) {
            xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (isbn->cur_agency != NULL && strcmp(isbn->cur_agency, (char *)key) != 0) {
                free(isbn->cur_agency);
                isbn->cur_agency = NULL;
            }
            if (isbn->cur_agency == NULL) {
                isbn->cur_agency = strdup((char *)key);
            }
            vprintl_kv("Agency", (char *)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"Rules"))) {
            parseRules(isbn, doc, cur);
        }
        cur = cur->next;
    }

    if (isbn->cur_prefix == NULL) {
        eprint("No prefix for Group ");
        fprint_path(eprint_fh, isbn);
        eprintl("");
        group_err = 1;
        isbn->cur_prefix = strdup("<PREFIX_ERROR>");
    }

    if (isbn->cur_agency == NULL) {
        eprint("No agency for Group ");
        fprint_path(eprint_fh, isbn);
        eprintl("");
        group_err = 1;
        isbn->cur_agency = strdup("<AGENCY_ERROR>");
    }

    add_prefix(isbn, isbn->cur_prefix, isbn->cur_agency);

    pop_path(isbn);
    return (group_err);
}

int
parseRegistrationGroups(isbn_info_t *isbn, xmlDocPtr doc, xmlNodePtr cur)
{
    int rgroup_err = 0;

    push_path(isbn, "RegistrationGroups");
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        int rc;

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"Group"))) {
            rc = parseGroup(isbn, doc, cur);
            if (rc) {
                rgroup_err = 1;
            }
        }
        cur = cur->next;
    }
    pop_path(isbn);
    return (rgroup_err);
}

static int
parseDoc(isbn_info_t *isbn, const char *docname)
{
    xmlDocPtr doc;
    xmlNodePtr cur;
    int doc_err = 0;

    isbn->cur_value = 0;

    doc = xmlParseFile(docname);

    if (doc == NULL ) {
        eprintl("Document not parsed successfully.");
        return (2);
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        eprintl("empty document");
        xmlFreeDoc(doc);
        return (2);
    }

    if (xmlStrcmp(cur->name, (const xmlChar *)"ISBNRangeMessage")) {
        eprintl("document of the wrong type, root node != ISBNRangeMessage");
        xmlFreeDoc(doc);
        return (2);
    }

    push_path(isbn, "ISBNRangeMessage");
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        int rc;

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"RegistrationGroups"))) {
            rc = parseRegistrationGroups(isbn, doc, cur);
            if (rc) {
                doc_err = 1;
            }
        }
        cur = cur->next;
    }

    pop_path(isbn);
    xmlFreeDoc(doc);
    return (doc_err);
}


// #################### Hyphenate a 13-digit ISBN

uint32_t
parse_number_slice(const char *s, size_t len)
{
    // XXX len must be < 22
    // XXX s must be all numeric

    uint32_t n = 0;
    while (len != 0) {
        n *= 10;
        n += *s - '0';
        ++s;
        --len;
    }

    return (n);
}


// #################### Hyphenate a 13-digit ISBN

/*
 * See: https://www.isbn-international.org/range_file_generation
 *
 * Anatomy of an ISBN
 * ------------------
 *
 * 1. prefix      := [EAN.UCC prefix]
 * 2. Group       := [Registration Group element]
 * 3. Registrant  := [Registrant element]
 * 4. Publication := [Publication element]
 * 5. Check       := [Check-digit]
 *
 * ISBN := prefix - Group - Registrant - Publication - Check
 *
 * Range table
 *
 * Offset
 * ------
 *           111
 * 0123456789012
 *     0123456
 *     |
 * These 7 digits are what is compared to entries in the range table.
 * Although the prefix is variable-length, and the dividing line
 * between { Group - Registrant - Publication } varies,
 * the combination of these 3 fields always starts at offset 4,
 * and is 7 digits long, and the range table entries tell how
 * these 3 fields are to be split.
 *
 * Using range tables allows for splitting Registration Groups
 * into more fine-grain pieces that could be achieved using more
 * prefix logic.
 *
 */

/*
 * Take a pure numeric ISBN-13 and specifications for where hyphens go,
 * and build a hyphenated ISBN-13 in the given buffer.
 *
 * @param  hbuf   out  address of destination buffer
 * @param  bsz    in   capacity of the destination buffer.
 *                     must be at least 18 bytes, 13 + hyphens + nul
 * @param  isbn   in   The pure numeric ISBN-13.
 * @param  len    in   The length of the 3rd field,
 *                     as specified in the range table for the given prefix.
 * @param  prefix in   The (variable-lengh) prefix, as it was parsed
 *                     by the finite-state transducer.
 * @param  pfxlen in   The length of |prefix|.
 */

void
place_hyphens(
  char *hbuf,
  size_t bsz,
  const char *isbn,
  size_t len,
  const char *prefix,
  size_t pfxlen)
{
    size_t lbuf;
    size_t l1;

    if (bsz < 18) {
        abort();
    }

    memcpy(hbuf, isbn, 3);      // Prefix
    lbuf = 3;
    hbuf[lbuf++] = '-';
    l1 = pfxlen - 3;
    memcpy(hbuf + lbuf, prefix + 3, l1);  // Registration Group
    lbuf += l1;
    hbuf[lbuf++] = '-';
    memcpy(hbuf + lbuf, isbn + 3 + l1, len);  // Registrant
    lbuf += len;
    hbuf[lbuf++] = '-';
    memcpy(hbuf + lbuf, isbn + 3 + l1 + len, 8 - len); // Publication
    lbuf += 8 - len;
    hbuf[lbuf++] = '-';
    memcpy(hbuf + lbuf, isbn + 12, 1); // Check-digit
    ++lbuf;
    hbuf[lbuf] = '\0';
}

/*
 * Figure out how to hyphenate a pure numeric 13-digit ISBN.
 * Find the correct prefix.
 * Since the prefix is variable-length we use a finite-state transducer (FST)
 * to know when to stop trying to match, and then to return an index
 * into the list of range tables.
 *
 * Then, find the appropriate range in the range table associated
 * with the prefix.
 *
 * Then, use the specifications in that range table entry
 * to build a hyphenated ISBN-13 by inserting hyphens in all the right places.
 *
 * @param   fst      in   Adrress of Finite-state transducer.
 * @param   hbuf     out  Address of the string-builder for the result.
 * @param   bsz      in   The capacity of |hbuf|.
 * @param   isbn     in   The pure numeric ISBN-13 to be hyphenated (zstring).
 *
 * @return  0 for success, non-zero for error codes.
 */

int
hyphenate_isbn(
  isbn_info_t *isbn,
  char *hbuf,
  size_t bsz,
  const char *isbn_str)
{
    fst_t *fst;
    size_t val;
    int rc;

    if (isbn == NULL) {
        return (ENODATA);
    }

    fst = isbn->fst;

    if (fst == NULL) {
        return (ENODATA);
    }

    // Need space for ISBN-13 + 4 hyphens + nul-byte
    if (bsz < 18) {
        return (ENOSPC);
    }

    rc = fst_lookup_prefix(fst, isbn_str, &val);
    if (rc) {
        if (verbose) {
            fprintf(vprint_fh, "Lookup of ('%s') failed; rc = %d.", isbn_str, rc);
            fprintl(vprint_fh, "");
        }
        return (rc);
    }

    isbn_prefix_t *prefix_tbl = isbn->prefix_vec.base;
    isbn_prefix_t *pfx = prefix_tbl + val;

    if (verbose) {
        fprintf(vprint_fh, "isbn %s -> prefix=%zu='%s'",
                isbn_str, val, pfx->prefix);
        fprintl(vprint_fh, "");
        fprintf(vprint_fh, "Agency='%s'", pfx->agency);
        fprintl(vprint_fh, "");
    }

    isbn_range_t *ranges_base;
    isbn_range_t *ranges_rule;
    size_t pfxlen;
    size_t n;

    pfxlen = strlen(pfx->prefix);
    ranges_base = isbn->ranges_vec.base;
    ranges_rule = ranges_base + pfx->rule_idx;
    n = pfx->nrules;
    uint32_t registrant = parse_number_slice(isbn_str + 4, 7);

    if (verbose) {
        // Show range table entries for this agency,
        // and show which range, if any, matches the given registrant

        size_t i;
        eprintf("registrant=%u", registrant);
        eprintl("");
        for (i = 0; i < n; ++i) {
            uint32_t lo = ranges_rule[i].rng_lbound;
            uint32_t hi = ranges_rule[i].rng_ubound;
            size_t len  = ranges_rule[i].rng_len;

            eprintf("    lo=%u, hi=%u, len=%zu", lo, hi, len);
            if (registrant >= lo && registrant <= hi) {
                eprint(" ==");
            }
            eprintl("");
        }
    }

    // Linear search for the registrant in range table for this agency

    size_t i;
    for (i = 0; i < n; ++i) {
        uint32_t lo = ranges_rule[i].rng_lbound;
        uint32_t hi = ranges_rule[i].rng_ubound;
        size_t len  = ranges_rule[i].rng_len;

        if (registrant >= lo && registrant <= hi) {
            place_hyphens(hbuf, bsz, isbn_str, len, pfx->prefix, pfxlen);
            return (0);
        }
    }

    return (ENOENT);
}

isbn_info_t *
parse_isbn_range_table(const char *docname)
{
    isbn_info_t *isbn;
    fst_t *isbn_prefix_fst_builder;

    isbn = new_isbn_info();
    parseDoc(isbn, docname);
    // XXX *result_fst = isbn_prefix_fst_builder;
    isbn_prefix_fst_builder = isbn->fst;
    isbn->fst = fst_copy_and_pack(isbn->fst);
    if (verbose) {
        fdump_prefix_table(vprint_fh, &isbn->prefix_vec);
        fflush(vprint_fh);
        fprintl(vprint_fh, "");
        fprintf(vprint_fh, "@section fst -- prefix state machine");
        fprintl(vprint_fh, "");
        fdump_fst(vprint_fh, isbn->fst);
        fprintl(vprint_fh, "@end fst");
        fdump_ranges(vprint_fh, &isbn->ranges_vec);
    }
    return (isbn);
}
