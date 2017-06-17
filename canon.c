#include <stdio.h>
#include "canon.h"
#include "tree.h"
#include "temp.h"
#include "util.h"
#include "symbol.h"

typedef struct expReferenceList_ *expReferenceList;
struct expReferenceList_
{
	TR_exp *head;
	expReferenceList tail;
};


static TR_stm do_stm(TR_stm stm);
static struct stm_exp do_exp(TR_exp e);
static TR_stmList getNext();
static CA_stmListList createBlocks(TR_stmList stms, TMP_label exit);

static expReferenceList ExpReferenceList(TR_exp *head, expReferenceList tail) {
	expReferenceList ptr = check_malloc(sizeof(*ptr));
	ptr->head = head;
	ptr->tail = tail;
	return ptr;
}
static bool isNop(TR_stm stm) {
	return stm->kind == TR_EXP && stm->u.EXP->kind == TR_CONST;
}
static TR_stm seq(TR_stm m, TR_stm n) {
	if (isNop(m)) return m;
	if (isNop(n)) return n;
	return TR_Seq(m, n);
}

static bool isCommute(TR_stm s, TR_exp e) {
	if (isNop(s)) {
		return TRUE;
	}
	if (e->kind == TR_NAME || e->kind == TR_CONST) {
		return TRUE;
	}
	return FALSE;
}

struct stm_exp
{
	TR_stm s;
	TR_exp e;
};
static struct stm_exp Stm_Exp(TR_stm s, TR_exp e) {
	struct stm_exp temp;
	temp.e = e;
	temp.s = s;
	return temp;
}

static TR_stm reorder(expReferenceList list) {
	if (list == NULL) return TR_Exp(TR_Const(0));
	else if ((*list->head)->kind == TR_CALL) {
		TMP_temp t = TMP_newtemp();
		*list->head = TR_Eseq(TR_Move(TR_Temp(t), *list->head), TR_Temp(t));
		return reorder(list);
	}
	else {
		struct stm_exp stmexp = do_exp(*list->head);
		TR_stm s = reorder(list->tail);
		if (isCommute(s, stmexp.e)) {
			*list->head = stmexp.e;
			return seq(stmexp.s, s);
		}
		else {
			TMP_temp t = TMP_newtemp();
			*list->head = TR_Temp(t);
			return seq(stmexp.s, seq(TR_Move(TR_Temp(t), stmexp.e), s));
		}
	}
}

static expReferenceList getCallList(TR_exp e) {
	expReferenceList list, cur;
	TR_expList args = e->u.CALL.args;
	cur = list = ExpReferenceList(&e->u.CALL.fun, NULL);
	for (; args; args = args->tail) {
		cur->tail = ExpReferenceList(&args->head, NULL);
		cur = cur->tail;
	}
	return list;
}

static struct stm_exp do_exp(TR_exp e) {
	switch (e->kind)
	{
	case TR_BINOP:
		return Stm_Exp(reorder(ExpReferenceList(&e->u.BINOP.left, ExpReferenceList(&e->u.BINOP.right, NULL))), e);
	case TR_MEM:
		return Stm_Exp(reorder(ExpReferenceList(&e->u.MEM, NULL)), e);
	case TR_ESEQ:
	{
		struct stm_exp temp = do_exp(e->u.ESEQ.exp);
		return Stm_Exp(seq(do_stm(e->u.ESEQ.stm), temp.s), temp.e);
	}
	case TR_CALL:
		return Stm_Exp(reorder(getCallList(e)), e);
	default:
		return Stm_Exp(reorder(NULL), e);
	}
}
/* remove eseq*/
static TR_stm do_stm(TR_stm stm) {
	switch (stm->kind)
	{
	case TR_SEQ:
		return seq(do_stm(stm->u.SEQ.left), do_stm(stm->u.SEQ.right));
	case TR_JUMP:
		return seq(reorder(ExpReferenceList(&stm->u.JUMP.exp, NULL)), stm);
	case TR_CJUMP:
		return seq(reorder(ExpReferenceList(&stm->u.CJUMP.left, ExpReferenceList(&stm->u.CJUMP.right, NULL))), stm);
	case TR_MOVE:
		if (stm->u.MOVE.dst->kind == TR_TEMP && stm->u.MOVE.src->kind == TR_CALL) {
			return seq(reorder(getCallList(stm->u.MOVE.src)), stm);
		}
		else if (stm->u.MOVE.dst->kind == TR_TEMP) {
			return seq(reorder(ExpReferenceList(&stm->u.MOVE.src, NULL)), stm);
		}
		else if (stm->u.MOVE.dst->kind == TR_MEM) {
			return seq(reorder(ExpReferenceList(&stm->u.MOVE.dst->u.MEM, ExpReferenceList(&stm->u.MOVE.src, NULL))), stm);
		}
		else if (stm->u.MOVE.dst->kind == TR_ESEQ) {
			TR_stm stm0 = stm->u.MOVE.dst->u.ESEQ.stm;
			stm->u.MOVE.dst = stm->u.MOVE.dst->u.ESEQ.exp;
			return do_stm(TR_Seq(stm0, stm));
		}
		assert(0);
	case TR_EXP:
		if (stm->u.EXP->kind == TR_CALL) {
			if (stm->u.EXP->kind == TR_CALL) {
				return seq(reorder(getCallList(stm->u.EXP)), stm);
			}
			else {
				return seq(reorder(ExpReferenceList(&stm->u.EXP, NULL)), stm);
			}
		}
	default:
		return stm;
	}
}

/* get rid of the top-level seqs*/
static TR_stmList linear(TR_stm stm, TR_stmList right) {
	if (stm->kind == TR_SEQ) {
		return linear(stm->u.SEQ.left, linear(stm->u.SEQ.right, right));
	}
	else {
		return TR_StmList(stm, right);
	}
}

TR_stmList CA_linearize(TR_stm stm)
{
	return linear(do_stm(stm), NULL);
}

static CA_stmListList StmListList(TR_stmList head, CA_stmListList tail) {
	CA_stmListList ptr = check_malloc(sizeof(*ptr));
	ptr->head = head;
	ptr->tail = tail;
	return ptr;
}
static CA_stmListList next(TR_stmList prev, TR_stmList cur, TMP_label exit) {
	if (cur == NULL) {
		return next(
			prev,
			TR_StmList(
				TR_Jump(
					TR_Name(exit),
					TMP_LabelList(exit, NULL)
				),
				NULL
			),
			exit
		);
	}
	else if (cur->head->kind == TR_JUMP || cur->head->kind == TR_CJUMP) {
		CA_stmListList stmLists;
		prev->tail = cur;
		stmLists = createBlocks(cur->tail, exit);
		cur->tail = NULL;
		return stmLists;
	}
	else if (cur->head->kind == TR_LABEL) {
		TMP_label label = cur->head->u.LABEL;
		return next(
			prev,
			TR_StmList(
				TR_Jump(
					TR_Name(label),
					TMP_LabelList(label, NULL)
				),
				cur
			),
			exit
		);
	}
	else {
		prev->tail = cur;
		return next(cur, cur->tail, exit);
	}
}
static CA_stmListList createBlocks(TR_stmList stms, TMP_label exit) {
	if (stms == NULL) {
		return NULL;
	}
	else if (stms->head->kind != TR_LABEL) {
		return createBlocks(TR_StmList(TR_Label(TMP_newlabel()), stms), exit);
	}
	else {
		return StmListList(stms, next(stms, stms->tail, exit));
	}
}
struct CA_block CA_basicBlocks(TR_stmList stmList)
{
	struct CA_block block;
	block.label = TMP_newlabel();
	block.stmLists = createBlocks(stmList, block.label);

	return block;
}

static SB_table blockEV;
static struct CA_block globalblock;

static TR_stmList getLast(TR_stmList list) {
	TR_stmList cur = list;
	while (cur->tail->tail)
	{
		cur = cur->tail;
	}
	return cur;
}
static void trace(TR_stmList list) {
	TR_stmList last = getLast(list);
	TR_stm label = list->head;
	TR_stm stm = last->tail->head;
	SB_enter(blockEV, label->u.LABEL, NULL);
	if (stm->kind == TR_JUMP) {
		TR_stmList targetList = SB_look(blockEV, stm->u.JUMP.jumps->head);
		if(stm->u.JUMP.jumps->tail == NULL && targetList) {
			last->tail = targetList;
			trace(targetList);
		}
		else {
			last->tail->tail = getNext();
		}
	}
	else if (stm->kind == TR_CJUMP) {
		TR_stmList trueBranch = SB_look(blockEV, stm->u.CJUMP.true);
		TR_stmList falseBranch = SB_look(blockEV, stm->u.CJUMP.false);
		if (falseBranch) {
			last->tail->tail = falseBranch;
		}
		else if (trueBranch) {
			last->tail->head = TR_Cjump(
				TR_notRel(stm->u.CJUMP.op),
				stm->u.CJUMP.left,
				stm->u.CJUMP.right,
				stm->u.CJUMP.false,
				stm->u.CJUMP.true
			);
			last->tail->tail = trueBranch;
			trace(trueBranch);
		}
		else {
			TMP_label falseBranch = TMP_newlabel();
			last->tail->head = TR_Cjump(
				stm->u.CJUMP.op,
				stm->u.CJUMP.left,
				stm->u.CJUMP.right,
				stm->u.CJUMP.true,
				falseBranch
			);
			last->tail->tail = TR_StmList(TR_Label(falseBranch), getNext());
		}

	}
	else
		assert(0);
}

static TR_stmList getNext() {
	if (!globalblock.stmLists) {
		return TR_StmList(TR_Label(globalblock.label), NULL);
	}
	else {
		TR_stmList stm = globalblock.stmLists->head;
		if (SB_look(blockEV, stm->head->u.LABEL)) {
			trace(stm);
			return stm;
		}
		else {
			globalblock.stmLists = globalblock.stmLists->tail;
			return getNext();
		}
	}
}

TR_stmList CA_traceaGeneration(struct CA_block block)
{
	CA_stmListList list;
	blockEV = SB_empty();
	globalblock = block;

	for (list = globalblock.stmLists; list; list = list->tail) {
		SB_enter(blockEV, list->head->head->u.LABEL, list->head);
	}

	return getNext();
}
