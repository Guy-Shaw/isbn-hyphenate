
/*
 * Filename: fprint.h
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

#ifndef _FPRINT_H
#define _FPRINT_H

#ifdef  __cplusplus
extern "C" {
#endif

#define vprint_fh eprint_fh

extern FILE *eprint_fh;
extern FILE *dprint_fh;

extern bool verbose;
extern bool debug;

extern void fprint(FILE *f, const char *str);
extern void fprintl(FILE *f, const char *str);
extern void fprint_kv(FILE *f, const char *k, const char *v);
extern void fprintl_kv(FILE *f, const char *k, const char *v);
extern void printl(const char *str);
extern void print(const char *str);
extern void eprintl(const char *str);

#if 0
extern void eprint(const char *str);
#endif

extern void vprintl(const char *str);
extern void vprint(const char *str);
extern void vprint_kv(const char *k, const char *v);
extern void vprintl_kv(const char *k, const char *v);
extern void dbg_printl(const char *str);

#if 0
extern void dbg_print(const char *str);
#endif

extern void dbg_show_var(const char *var_id, const char *var_value);

#ifdef  __cplusplus
}
#endif

#endif  /* _FPRINT_H */
