
/*
 * Filename: fprint.c
 * Library: libcscript
 * Brief: A somewhat more rational alternative to fputs(), fprintf(), etc.
 *
 * Copyright (C) 2015-2020 Guy Shaw
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

#include <stdbool.h>
    // Import type bool
#include <stdio.h>
    // Import constant EOF
    // Import type FILE
    // Import fputs()
    // Import var stderr
    // Import var stdout
#include <stdlib.h>
    // Import abort()

#include <fprint.h>

static const char *endl = "\n";

extern FILE *errprint_fh;
extern FILE *dbgprint_fh;

extern bool verbose;
extern bool debug;

void
fprint(FILE *f, const char *str)
{
    int rc;

    rc = fputs(str, f);
    if (rc == EOF) {
        (void) fputs("I/O error\n", eprint_fh);
        abort();
    }
}

void
fprintl(FILE *f, const char *str)
{
    fprint(f, str);
    fprint(f, endl);
}

void
fprint_kv(FILE *f, const char *k, const char *v)
{
    fprint(f, k);
    fprint(f, ": ");
    fprint(f, v);
}

void
fprintl_kv(FILE *f, const char *k, const char *v)
{
    fprint_kv(f, k, v);
    fprintl(f, "");
}

void
printl(const char *str)
{
    fprintl(stdout, str);
}

void
print(const char *str)
{
    fprint(stdout, str);
}

void
eprintl(const char *str)
{
    fprintl(eprint_fh, str);
}

#if 0
void
eprint(const char *str)
{
    fprint(eprint_fh, str);
}
#endif

void
vprintl(const char *str)
{
    if (verbose) {
        fprintl(vprint_fh, str);
    }
}

void
vprint(const char *str)
{
    if (verbose) {
        fprint(vprint_fh, str);
    }
}

void
vprint_kv(const char *k, const char *v)
{
    if (verbose) {
        fprint_kv(vprint_fh, k, v);
    }
}

void
vprintl_kv(const char *k, const char *v)
{
    if (verbose) {
        fprintl_kv(vprint_fh, k, v);
    }
}

void
dbg_printl(const char *str)
{
    if (debug) {
        fprintl(dprint_fh, str);
    }
}

#if 0
void
dbg_print(const char *str)
{
    if (debug) {
        fprint(dprint_fh, str);
    }
}
#endif

void
dbg_show_var(const char *var_id, const char *var_value)
{
    fprint(dprint_fh, var_id);
    fprint(dprint_fh, "=[");
    fprint(dprint_fh, var_value);
    fprint(dprint_fh, "]");
    fprint(dprint_fh, endl);
}
