#ifndef _CANON_H_
#define _CANON_H_
#include "tree.h"
typedef struct CA_stmListList_ *CA_stmListList;
struct CA_block
{
	CA_stmListList stmLists;
	TMP_label label;
};
struct CA_stmListList_
{
	TR_stmList head;
	CA_stmListList tail;
};

/* produce a list of cleaned trees satisfying the following properties:
       1. no seq or eseq
	   2. the parent of every call is an exp(..) or a mome(tmp t, ..)
*/
TR_stmList CA_linearize(TR_stm stm);

/* generate basic blocks, each of which begins with a label, ends with JUMP or CJUMP
*/
struct CA_block CA_basicBlocks(TR_stmList stmList);

/* divide blocks into some traces
*/
TR_stmList CA_traceaGeneration(struct CA_block block);
#endif // !_CANON_H_
