/*
 * Filename: src/cmd/isbn-hyphenate
 * Project: isbn-hyphenate
 * Brief: Hyphenate ISBNs using the range table from International ISBN Agency
 *
 * Copyright (C) 2016-2019 Guy Shaw
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <ctype.h>
    // Import isprint()
#include <err.h>
    // Import err()
#include <errno.h>
    // Import var errno
#include <stdbool.h>
    // Import type bool
    // Import constant false
    // Import constant true
#include <stddef.h>
    // Import constant NULL
#include <stdio.h>
    // Import type FILE
    // Import fclose()
    // Import fopen()
    // Import fprintf()
    // Import fputc()
    // Import fputs()
    // Import snprintf()
    // Import var stdin
    // Import var stdout
#include <stdlib.h>
    // Import exit()
    // Import free()
#include <string.h>
    // Import strdup()
#include <unistd.h>
    // Import getopt_long()
    // Import optarg()
    // Import opterr()
    // Import optind()
    // Import optopt()
    // Import type size_t

#include <cscript.h>
#include <fprint.h>
#include <isbn-info.h>
#include <libfst.h>

extern isbn_info_t *parse_isbn_range_table(const char *docname);
extern int hyphenate_isbn(isbn_info_t *, char *hbuf, size_t bsz, const char *isbn);

const char *program_path;
const char *program_name;

size_t filec;               // Count of elements in filev
char **filev;               // Non-option elements of argv

FILE *eprint_fh = NULL;
FILE *dprint_fh = NULL;

static isbn_info_t *isbn_info;

bool verbose  = false;
bool debug    = false;
bool opt_argv = false;

static struct option long_options[] = {
    {"help",     no_argument, 0,'h'},
    {"version",  no_argument, 0,'V'},
    {"verbose",  no_argument, 0,'v'},
    {"debug",    no_argument, 0,'d'},
    {"argv",     no_argument, 0,'A'},
    {0, 0, 0, 0}
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?         Show this help message and exit\n"
    "  --version            Show version information and exit\n"
    "  --verbose|-v         verbose\n"
    "  --debug|-d           debug\n"
    "  --argv               Input is argv, instead of from files\n"
    ;

static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016-2019 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_program_version(FILE *f)
{
    fprintl(f, version_text);
    fprintl(f, copyright_text);
    fprintl(f, license_text);
}

static void
show_program_version(void)
{
    fshow_program_version(stdout);
}

static void
usage(void)
{
    // XXX Show program name using libcscript function
    // XXX to do quoting and translating all characters to graphic.

    eprint("usage: ");
    eprint(program_name);
    eprintl(" [ <options> ]");
    eprint(usage_text);
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

// ################ linebuf

struct linebuf {
    FILE   *f;
    char   *buf;
    void   *sgl;
    size_t siz;
    size_t len;
    int    err;
    bool   eof;
};

typedef struct linebuf linebuf_t;

extern linebuf_t *linebuf_new(void);
extern void linebuf_init(linebuf_t *lbuf, FILE *f);
extern void linebuf_sgl_new(linebuf_t *lbuf);
extern void linebuf_free(linebuf_t *lbuf);
extern char *sgl_fgetline(linebuf_t *lbuf, int endl);

static void
fgetline(linebuf_t *lbuf)
{
    char *rbuf;

    // Free up any resources leftover from the last time
    // this line buffer was used.
    //
    // In particlur, free up any segments of a scatter/gather list
    // and the line buffer.

    linebuf_free(lbuf);

    linebuf_sgl_new(lbuf);

    rbuf = sgl_fgetline(lbuf, '\n');
    (void)rbuf;
}

int
isbn_stream(const char *fname, FILE *f)
{
    (void)fname;    // We will use later for error reporting.

    linebuf_t *lbuf;
    char hbuf[32];
    int rv;

    lbuf = linebuf_new();
    linebuf_init(lbuf, f);
    while (true) {
        fgetline(lbuf);
        if (lbuf->eof) {
            break;
        }
        if (lbuf->len == 0) {
            continue;
        }

        dbg_show_var("line=[%s]", lbuf->buf);

        rv = hyphenate_isbn(isbn_info, hbuf, sizeof (hbuf), lbuf->buf);
        if (rv == 0) {
            printl(hbuf);
        }
    }
    linebuf_free(lbuf);
    free(lbuf);

    return (0);
}

int
filev_isbn(void)
{
    size_t fnr;

    for (fnr = 0; fnr < filec; ++fnr) {
        FILE *f;
        int rv;
        int close_rv;

        f = fopen(filev[fnr], "r");
        if (f == NULL) {
            int err = errno;
            // XXX Use libcscript::explain_errno();
            eprintf("fopen('%s', \"r\") failed\n", filev[fnr]);
            eprintf("  errno=%d\n", err);
            return (err);
        }

        rv = isbn_stream(filev[fnr], f);
        close_rv = fclose(f);
        if (rv) {
            eprintf("isbn_stream('%s') failed.\n", filev[fnr]);
            return (rv);
        }
        if (close_rv) {
            eprintf("fclose('%s') failed.\n", filev[fnr]);
            return (rv);
        }
    }

    return (0);
}

int
argv_isbn(size_t argc, char **argv)
{
    char hbuf[32];
    size_t i;
    int rv;

    for (i = 0; i < argc; ++i) {
        rv = hyphenate_isbn(isbn_info, hbuf, sizeof (hbuf), argv[i]);
        if (rv == 0) {
            printl(hbuf);
        }
    }

    return (0);
}

int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    int rv;

    set_eprint_fh();
    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "+hVdvEHw:", long_options, &option_index);
        if (optc == -1) {
            break;
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_program_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            set_debug_fh(NULL);
            break;
        case 'v':
            verbose = true;
            break;
        case 'A':
            opt_argv = true;
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, '%c'\n",
                program_name, optopt);
            exit(2);
            break;
        }
    }

    verbose = verbose || debug;

    if (optind < argc) {
        filec = (size_t) (argc - optind);
        filev = argv + optind;
    }
    else {
        filec = 0;
        filev = NULL;
    }

    if (verbose) {
        fshow_str_array(eprint_fh, filec, filev);
    }

    if (verbose && optind < argc) {
        eprintl("non-option ARGV-elements:");
        while (optind < argc) {
            eprint("    ");
            eprintl(argv[optind]);
            ++optind;
        }
    }

    if (err_count != 0) {
        usage();
        exit(1);
    }

    isbn_info = (void *)parse_isbn_range_table("isbn-range.xml");
    if (rv != 0) {
        exit(rv);
    }

    if (opt_argv) {
        rv = argv_isbn(filec, filev);
    }
    else if (filec == 0) {
        rv = isbn_stream("-", stdin);
    }
    else {
        rv = filev_probe(filec, filev);
        if (rv != 0) {
            exit(rv);
        }

        rv = filev_isbn();
    }

    if (rv != 0) {
        exit(rv);
    }

    exit(0);
}
