#include <stdio.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "table.h"

struct S_symbol_ {string name; S_symbol next;};

static S_symbol mksymbol(string name, S_symbol next)
{
	S_symbol s=checked_malloc(sizeof(*s));
	s->name=name; s->next=next;
	if (Debug_JanathonL)
	{
		printf("s->name:%s\n", s->name);
	}
	return s;
}

#define SIZE 109  /* should be prime */

static S_symbol hashtable[SIZE];

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
S_symbol syms,sym;
S_symbol S_Symbol(string name)
{
	int index= hash(name) % SIZE;
	
	syms = hashtable[index];
	if (Debug_JanathonL)
	{
		static int cnt=0;
		cnt++;
		if (cnt>1)
		{
			printf("test syms\n");
			printf("syms->name:%s\n", syms->name);
			printf("hashtable[%d]->name:%s\n",index,hashtable[index]->name);
		}
		printf("hash_index:%d\n", index);
		//printf("syms->name:%s",syms? "none":syms->name);
	}
	
	for(sym=syms; sym; sym=sym->next)
		if (streq(sym->name,name)) {
			
			if (Debug_JanathonL)
			{
				printf("sym->name:%s\n",sym->name);
				printf("findSymbol!:%s\n", sym->name);
			}
			return sym;
		}
	
	sym = mksymbol(name,syms);

	hashtable[index]=sym;
	if (Debug_JanathonL)
	{
		printf("can not find Symbol:%s\n", name);
		printf("sym->name:%s\n", sym->name);
		printf("hashtable[%d]->name:%s\n",index,hashtable[index]->name);
	}
	return sym;
}
 
string S_name(S_symbol sym)
{
 return sym->name;
}

S_table S_empty(void) 
{ 
 return TAB_empty();
}

void S_enter(S_table t, S_symbol sym, void *value) {
  TAB_enter(t,sym,value);
}

void *S_look(S_table t, S_symbol sym) {
  return TAB_look(t,sym);
}

static struct S_symbol_ marksym = {"<mark>",0};

void S_beginScope(S_table t)
{ S_enter(t,&marksym,NULL);
}

void S_endScope(S_table t)
{S_symbol s;
  do s=TAB_pop(t);
  while (s != &marksym);
}

void S_dump(S_table t, void (*show)(S_symbol sym, void *binding)) {
  TAB_dump(t, (void (*)(void *, void *)) show);
}

