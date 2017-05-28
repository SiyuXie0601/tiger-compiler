/*********************************************************************************
*FileName: translate.h
*Author: Debbie Xie
*Create Time: 2017/5/24
*Description: IR Translation Module
**********************************************************************************/

#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

extern const int FRM_wordSize;


/* Stack Structure START */
typedef struct stack_item_ *stack_item;
struct stack_item_ {
	void *key;
	stack_item next;
};
void Stack_push(stack_item *stackList, void *key);
void Stack_pop(stack_item *stackList);
void Stack_empty(stack_item *stackList);
bool Stack_check(stack_item *stackList, void *key, bool (*compare)(void*, void*));
void* Stack_peek(stack_item *stackList);
/* Stack Structure END */

typedef struct TL_access_ *TL_access;

typedef struct TL_accessList_ *TL_accessList;
struct TL_accessList_ {	
	TL_access head;
	TL_accessList tail;
};
TL_accessList TL_AccessList(TL_access head, TL_accessList tail);

typedef struct TL_level_ *TL_level;

TL_level TL_outermost(void);
TL_level TL_newLevel(TL_level parent, TMP_label name, U_boolList formals);
TL_accessList TL_formals(TL_level level);
TL_access TL_allocLocal(TL_level level, bool escape);
TL_level TL_getParent(TL_level parent);
void TL_printLevel(TL_level level);

typedef struct patchList_ *patchList;
struct patchList_ {
	TMP_label *labelOnTree;
	patchList next;
};
static patchList PatchList(TMP_label *head, patchList tail);

typedef struct TL_exp_ *TL_exp;
typedef struct TL_expList_ *TL_expList;
struct TL_expList_ {
	TL_exp head;
	TL_expList tail;
};
TL_expList TL_ExpList(TL_exp head, TL_expList tail);

struct Cx {
	patchList trues; 
	patchList falses; 
	TR_stm stm;
};
struct TL_exp_ {
	enum {TL_ex, TL_nx, TL_cx} kind;
	union {
		TR_exp ex; 
		TR_stm nx; 
		struct Cx cx;
	} u;
};

//For debugging only
void TL_printTrExp(TL_exp te);
void TL_printResult();
void TL_printTree(TL_exp exp);

//Constructors
static TL_exp TL_Ex(TR_exp ex);
static TL_exp TL_Nx(TR_stm nx);
static TL_exp TL_Cx(patchList trues, patchList falses, TR_stm stm);

//Extractors
static TR_exp unEx(TL_exp e);
static TR_stm unNx(TL_exp e);
static struct Cx unCx(TL_exp e);

//Tranlation for expressions
TL_exp TL_nilExp();
TL_exp TL_assignExp(TL_exp lvalue, TL_exp right);
TL_exp TL_forExp(TL_exp var, TL_exp low, TL_exp high, TL_exp body);
TL_exp TL_whileExp(TL_exp condition, TL_exp body);
void TL_genLoopDoneLabel();
TL_exp TL_breakExp();
TL_exp TL_ifExp(TL_exp cond, TL_exp thenb, TL_exp elseb);
TL_exp TL_simpleVar(TL_access translate_acc, TL_translate_level);
TL_exp TL_arrayVar(TL_exp var, TL_exp offset_exp);
TL_exp TL_intExp(int int_variable);
TL_exp TL_stringExp(string str_variable);
TL_exp TL_arithExp(AST_oper operator, TL_exp leftExp, TL_exp rightExp);
TL_exp TL_logicExp(AST_oper oper, TL_exp left, TL_exp right, bool isStrCompare);
TL_exp TL_arrayExp(TL_exp size, TL_exp init);
TL_exp TL_recordExp_new(int cnt);
void TL_recordExp_app(TL_exp te, TL_exp init, bool last);
TL_exp TL_seqExp(TL_exp*, int);
TL_exp TL_voidExp(void);
TL_exp TL_callExp(TL_level caller_lvl, TL_level callee_lvl, TMP_label fun_label, TL_exp* argv, int args);
void TL_procEntryExit(TL_level level, TL_exp body, TL_accessList formals, TMP_label label);

//Fragment list
FRM_fragList TL_getResult(void);

#endif
