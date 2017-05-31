#include <stdio.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "table.h"

struct SB_symbol_ {string name; SB_symbol next;};

static SB_symbol mksymbol(string name, SB_symbol next)
{
	SB_symbol s=check_malloc(sizeof(*s));
	s->name=name; s->next=next;
	return s;
}

#define SIZE 109  /* should be prime */

static SB_symbol hashtable[SIZE];

static unsigned int hash(char *s0)
{unsigned int h=0; char *s;
 for(s=s0; *s; s++)  
       h = h*65599 + *s;
 return h;
}
 
static int streq(string a, string b)
{
 return !strcmp(a,b);
}
SB_symbol syms,sym;
SB_symbol SB_Symbol(string name)
{
	int index= hash(name) % SIZE;
	
	syms = hashtable[index];
	
	for(sym=syms; sym; sym=sym->next)
		if (streq(sym->name,name)) {
			return sym;
		}
	
	sym = mksymbol(name,syms);

	hashtable[index]=sym;
	return sym;
}
 
string SB_name(SB_symbol sym)
{
 return sym->name;
}

SB_table SB_empty(void) 
{ 
 return TB_create();
}

void SB_enter(SB_table t, SB_symbol sym, void *value) {
  TB_push(t,sym,value);
}

void *SB_look(SB_table t, SB_symbol sym) {
  return TB_look(t,sym);
}

static struct SB_symbol_ marksym = {"<mark>",0};

void SB_beginScope(SB_table t)
{ SB_enter(t,&marksym,NULL);
}

void SB_endScope(SB_table t)
{SB_symbol s;
  do s=TB_pop(t);
  while (s != &marksym);
}
/*
void SB_dump(SB_table t, void (*show)(SB_symbol sym, void *binding)) {
  TAB_dump(t, (void (*)(void *, void *)) show);
}
*/
