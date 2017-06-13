/*
 * symbol.h - Symbols and symbol-tables
 *
 */
#ifndef _SYMBOL_H_
#define _SYMBOL_H_
#include "util.h"
typedef struct SB_symbol_ * SB_symbol;

/* Make a unique symbol from a given string.  
 *  Different calls to SB_Symbol("foo") will yield the same SB_symbol
 *  value, even if the "foo" strings are at different locations. */
SB_symbol SB_Symbol(string);

/* Extract the underlying string from a symbol */
string SB_name(SB_symbol);

/* SB_table is a mapping from SB_symbol->any, where "any" is represented
 *     here by void*  */
typedef struct TB_table_ *SB_table;

/* Make a new table */
SB_table SB_empty(void);

/* Enter a binding "sym->value" into "t", shadowing but not deleting
 *    any previous binding of "sym". */
void SB_enter(SB_table t, SB_symbol sym, void *value);

/* Look up the most recent binding of "sym" in "t", or return NULL
 *    if sym is unbound. */
void *SB_look(SB_table t, SB_symbol sym);

/* Start a new "scope" in "t".  Scopes are nested. */
void SB_beginScope(SB_table t);

/* Remove any bindings entered since the current scope began,
   and end the current scope. */
void SB_endScope(SB_table t);

#endif
