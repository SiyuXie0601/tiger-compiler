/*************************************************************************
	> Created Time: 2017年05月23日 星期二 10时10分52秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "types.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "semant.h"

bool hasError = FALSE;

static struct exp_tp *empty_exp_tp; 
static TP_tp actual_tp(TP_tp tp);
static char string_operator[][4] = {
	"+",
	"-",
	"*",
	"/",
	"=",
	"<>",
	"<",
	"<=",
	">",
	">="
};
static bool looseTypeCompare(TP_tp m, TP_tp n);


static bool looseTypeCompare(TP_tp m, TP_tp n){
	TP_tp m0 = actual_tp(m);
	TP_tp n0 = actual_tp(n);

	if(m0 == n0){
		return TRUE;
	}

	return FALSE;
}

/*
 * loop variants stack
 */
static stack_item loop_var_stack = NULL; 
static void Loop_var_stack_push(SB_symbol sb);
static void Loop_var_stack_pop();
static bool Loop_var_stack_look(SB_symbol sb);
static bool comparePointer(void *m, void *n);

static void Loop_var_stack_push(SB_symbol sb){
	Stack_push(&loop_var_stack, sb);
}
static void Loop_var_stack_pop(){
	Stack_pop(&loop_var_stack);
}
static bool Loop_var_stack_look(SB_symbol sb){
	return Stack_check(&loop_var_stack, sb, comparePointer);
}
static bool comparePointer(void *m, void *n){
	return m == n ? TRUE : FALSE;
}

/*
 * type symbol stack
 */
static stack_item type_sym_stack = NULL;
static void Type_sym_stack_push(SB_symbol sb);
static void Type_sym_stack_pop();
static bool Type_sym_stack_look(SB_symbol sb);
static void Type_sym_stack_empty();

static void Type_sym_stack_push(SB_symbol sb){
	Stack_push(&type_sym_stack, sb);
}
static void Type_sym_stack_pop(){
	Stack_pop(&type_sym_stack);
}
static bool Type_sym_stack_look(SB_symbol sb){
	return Stack_check(&type_sym_stack, sb, comparePointer);
}
static void Type_sym_stack_empty(){
	Stack_empty(&type_sym_stack);
}

static TP_tp generateTp(AST_ty a){
	SB_symbol sb;
	TP_tp tp;
	switch(a->kind){
		case AST_arrayTy:
			return TP_Array(NULL);
		default:;
	}
}


/*
 * error dealing
 */
typedef enum {WARNING, ERROR, SEVERE, FATAL} sm_error_level;
static int SM_alert_level = -1;
static char string_sm_error_level[][15] = {
	"[warning] ",
	"[error] ",
	"[error] ",
	"[error] "
};
static int str_length_sm_error_level[] = {
	10,
	8,
	8,
	8
};
static void SM_info(sm_error_level level, int pos, char *info, ...);
static struct exp_tp SM_check_exit(sm_error_level level);

static void SM_info(sm_error_level level, int pos, char *info, ...){
	va_list arglist;
	va_start(arglist, info);
	char str0[128];
	char *str = str0;

	strcpy(str, string_sm_error_level[level]);
	str = str + str_length_sm_error_level[level];
	vsprintf(str, info, arglist);
	va_end(arglist);
	ERRMSG_error(pos, str0);

	if(SM_alert_level < (int)level){
		SM_alert_level = (int)level;
	}
	if(level == FATAL){
		exit(1);
	}
}
static struct exp_tp SM_check_exit(sm_error_level level){
	if(SM_alert_level >= (int)level){
		exit(1);
	}
	return *empty_exp_tp;
}


static void transTp(SB_table typeEV, SB_symbol sb, AST_ty a){
	SB_symbol sym;
	TP_tp tp;
	
	TP_tp cur_tp = SB_look(typeEV, sb);
	switch (a->kind){
		case AST_arrayTy:
			sym = a->u.array;
			tp = SB_look(typeEV, sym);
			if(tp == NULL){
				SM_info(FATAL, a->pos, "undefined type:'%s'", SB_name(sym));
			}
			else{
				cur_tp->u.array = tp;
			}
			break;
	}
}

static TP_tpList generateFormalTpList(SB_table typeEV, AST_fieldList params) {
	TP_tpList tpList = TP_TpList(NULL, NULL);
	TP_tpList tpList_head = tpList;
	bool isLegal = TRUE;

	SB_symbol sym;
	TP_tp tp;

	Type_sym_stack_empty();

	for (; params != NULL; params = params->tail) {
		AST_field head = params->head;
		if (head == NULL) {
			continue;
		}

		if (Loop_var_stack_look(head->name)) {
			SM_info(SEVERE, head->pos, "the name of loop variable '%s' cannot be reused by function's argument", SB_name(head->name));
		}

		sym = head->typ;
		tp = SB_look(typeEV, sym);
		if (tp == NULL) {
			SM_info(SEVERE, head->pos, "undefined type: '%s'", SB_name(sym));
			isLegal = FALSE;
		}

		if (!Type_sym_stack_look(head->name)) {
			Type_sym_stack_push(head->name);
		}
		else {
			SM_info(SEVERE, head->pos, "the name of formal argument '%s' has been used for the function", SB_name(head->name));
		}

		if (isLegal) {
			tpList->tail = TP_TpList(NULL, NULL);
			tpList->head = tp;
			tpList = tpList->tail;
		}
	}

	tpList->tail = NULL;

	SM_check_exit(SEVERE);

	if (tpList_head->head != NULL) {
		return tpList_head;
	}
	else {
		return NULL;
	}
}

static void generateEscapeList(UN_boolList *formal_escapes, AST_fieldList fieldList) {
	if (fieldList == NULL) {
		*formal_escapes = NULL;
		return;
	}

	UN_boolList escapes = NULL;
	UN_boolList escapes_head = NULL;

	for (; fieldList != NULL; fieldList = fieldList->tail) {
		if (escapes_head == NULL) {
			escapes = UN_BoolList(TRUE, NULL);
			escapes_head = escapes;
		}
		else {
			escapes->tail = UN_BoolList(TRUE, NULL);
			escapes = escapes->tail;
		}
	}
	if (escapes != NULL) {
		escapes->tail = NULL;
	}
	*formal_escapes = escapes_head;
}


void initialize(){
	empty_exp_tp = check_malloc(sizeof(*empty_exp_tp));
	empty_exp_tp->exp = TL_voidExp();
	empty_exp_tp->tp = TP_Void(); 
}

FRM_fragList SM_transProgram(AST_exp exp){
	initialize();

	SB_table valueEV = EV_base_valueEv();
	SB_table typeEV = EV_base_typeEv();

	TL_level rootLevel = TL_newLevel(TL_outermost(), TMP_namedlabel("_main_"), NULL);
	struct exp_tp result = transExp(rootLevel, valueEV, typeEV, exp);
	
	// print IR tree
	TL_printTree(result.exp);
	TL_procEntryExit(rootLevel, result.exp, TL_formals(rootLevel), TMP_newlabel());

	hasError = (SM_alert_level >= (int)ERROR)?TRUE:FALSE;
	
	return TL_getResult();
}
#define RIGHT_ARGUMENTS 0
#define LESS_ARGUMENTS 1
#define MORE_ARGUMENTS 2

struct exp_tp transExp(TL_level level, SB_table valueEV, SB_table typeEV, AST_exp e){
	int i;
	struct exp_tp exp_tp;
	AST_exp exp;
	TL_exp te;
	
	// AST_assignExp
	TP_tp tp;

	// AST_letExp
	AST_decList d;

	// AST_callExp
	TP_tpList formalTypes;
	AST_expList arguments;
	EV_item item;
	int argument_number;
	TL_exp* argument_exps;

	// AST_opExp
	struct exp_tp left_exp_tp;
	struct exp_tp right_exp_tp;
	TP_tp left_tp;
	TP_tp right_tp;
	AST_oper oper;
	bool isStringComparing;

	// AST_ifExp
	// AST_whileExp 
	// But AST_whileExp only has compare_exp
	struct exp_tp true_exp;
	struct exp_tp false_exp;
	struct exp_tp compare_exp;
	bool is_if_then_else;

	// AST_forExp
	// AST_whileExp
	SB_symbol var_sym;
	struct exp_tp for_start_exp;
	struct exp_tp for_end_exp;
	TL_access tl_access;
	struct exp_tp body_exp;
	struct exp_tp var_exp;

	// AST_seqExp
	AST_expList expList;
	TL_exp *te_list;

	// AST_arrayExp
	TP_tp arr_tp;
	struct exp_tp arr_exp_tp;
	
	if(e == NULL){
		return *empty_exp_tp;
	}

	/*
	 * AST_varExp
	 * AST_intExp
	 * AST_stringExp
	 * AST_opExp
	 * AST_seqExp
	 * AST_assignExp
	 * AST_ifExp
	 * AST_whileExp
	 * AST_forExp
	 * AST_letExp
	 * AST_arrayExp
	 */
	switch(e->kind){
		case AST_varExp:
			return transVar(level, valueEV, typeEV, e->u.var);
		case AST_intExp:
			te = TL_intExp(e->u.intt);
			return Exp_Tp(te, TP_Int());
		case AST_stringExp:
			te = TL_stringExp(e->u.stringg);
			return Exp_Tp(te, TP_String());
		case AST_callExp:
			argument_number = RIGHT_ARGUMENTS;

			// get function declaration from environment
			item = SB_look(valueEV, e->u.call.func);

			if (item != NULL && item->kind == EV_funItem) {
				// use an array to hold all the arguments
				i = 0;
				for (formalTypes = item->u.fun.formalTypes; formalTypes != NULL; formalTypes = formalTypes->tail) {
					if (formalTypes->head != NULL) {
						i++;
					}
				}
				argument_exps = NULL;
				if (i > 0) {
					argument_exps = check_malloc(i * sizeof(TL_exp));
				}

				i = 0;
				for (formalTypes = item->u.fun.formalTypes, arguments = e->u.call.args;
					formalTypes != NULL && arguments != NULL;
					formalTypes = formalTypes->tail, arguments = arguments->tail, i++) {
					tp = formalTypes->head;
					exp = arguments->head;
					if (exp != NULL && tp != NULL) {
						exp_tp = transExp(level, valueEV, typeEV, exp);
						if (actual_tp(exp_tp.tp) != actual_tp(tp)) {
							SM_info(SEVERE, exp->pos, "Type of argument %d passed to function '%s' is incompatible with the declared type", i + 1, SB_name(e->u.call.func));
						}
						argument_exps[i] = exp_tp.exp;
					}
					else if (exp == NULL && tp != NULL) {
						argument_number = LESS_ARGUMENTS;
						break;
					}
					else if(exp != NULL && tp == NULL){
						argument_number = MORE_ARGUMENTS;
						break;
					}
				}

				if (formalTypes != NULL && arguments == NULL) {
					if (formalTypes->head != NULL) {
						argument_number = LESS_ARGUMENTS;
					}
				}
				if (formalTypes == NULL && arguments != NULL) {
					if (arguments->head != NULL) {
						argument_number = MORE_ARGUMENTS;
					}
				}
				if (argument_number == LESS_ARGUMENTS) {
					SM_info(SEVERE, e->pos, "less than necessary arguments are passed to function '%s'", SB_name(e->u.call.func));
				}
				if (argument_number == MORE_ARGUMENTS) {
					SM_info(SEVERE, e->pos, "more than necessary arguments are passed to function '%s'", SB_name(e->u.call.func));
				}

				SM_check_exit(SEVERE);

				te = TL_callExp(level, TL_getParent(item->u.fun.level), item->u.fun.label, argument_exps, i);
				check_free(argument_exps);

				return Exp_Tp(te, item->u.fun.resultType);
			}
			else {
				SM_info(FATAL, e->pos, "undefined function: %s", SB_name(e->u.call.func));
				return SM_check_exit(FATAL);
			}
		case AST_opExp:
			left_exp_tp = transExp(level, valueEV, typeEV, e->u.op.left);
			right_exp_tp = transExp(level, valueEV, typeEV, e->u.op.right);
			left_tp = actual_tp(left_exp_tp.tp);
			right_tp = actual_tp(right_exp_tp.tp);
			oper = e->u.op.oper;

			switch (oper){
				/*
				 * for arithmetic expressions where operator is +, -, *, /
				 * result value must be integer
				 */
				case AST_plusOp:
				case AST_minusOp:
				case AST_timesOp:
				case AST_divideOp:
					if(left_tp->kind != TP_int){
						SM_info(SEVERE, e->pos, "Illegal operation: left side of '%s' operation is not an integer!", string_operator[oper]);
					}
					if(right_tp->kind != TP_int){
						SM_info(SEVERE, e->pos, "Illegal operation: right side of '%s' operation is not an integer!", string_operator[oper]);
					}
					// terminate program if given error has occured
					SM_check_exit(SEVERE);

					te = TL_arithExp(oper, left_exp_tp.exp, right_exp_tp.exp);
					return Exp_Tp(te, TP_Int());
				/*
				 * for comparison expressions where operator is <, >, <=, >=
				 * must be either integer or string
				 * both sides must be of the same type
				 */
				case AST_ltOp:
				case AST_leOp:
				case AST_gtOp:
				case AST_geOp:
					if(left_tp->kind != TP_int && left_tp->kind != TP_string){
						SM_info(SEVERE, e->pos, 
								"Illegal operaion: the left side of '%s' operation is not an integer or string!", string_operator[oper]);
					}
					if(right_tp->kind != TP_int && right_tp->kind != TP_string){
						SM_info(SEVERE, e->pos, 
								"Illegal operaion: the left side of '%s' operation is not an integer or string!", string_operator[oper]);
					}
					SM_check_exit(SEVERE);
					if(left_tp->kind != right_tp->kind){
						SM_info(SEVERE, e->pos, 
								"Illegal operaion: the two sides of '%s' operation are not of the same type!", 
								string_operator[oper]);
					}
					else{
						isStringComparing = left_tp->kind == TP_int?FALSE:TRUE;
						te = TL_logicExp(oper, left_exp_tp.exp, right_exp_tp.exp, isStringComparing);
						return Exp_Tp(te, TP_Int());
					}
					break;
				/*
				 * for comparison expressions where operator is =, <>:
				 * must be integer, string, record or array; the two sides must be of the same type;
				 */
				case AST_eqOp:
				case AST_neqOp:
					if (left_tp->kind != TP_int && 
						left_tp->kind != TP_string &&
						left_tp->kind != TP_array){
						SM_info(SEVERE, e->pos, "Illegal operation: the left side of '%s' operation is not one of the following types:integer, string, array", string_operator[oper]);
					}
					if (right_tp->kind != TP_int && 
						right_tp->kind != TP_string &&
						right_tp->kind != TP_array){
						SM_info(SEVERE, e->pos, "Illegal operation: the right side of '%s' operation is not one of the following types:integer, string, array", string_operator[oper]);
					}
					SM_check_exit(SEVERE);
					if(left_tp->kind != right_tp->kind){
						SM_info(FATAL, e->pos, 
								"Illegal operaion: the two sides of '%s' operation are not of the same type!", 
								string_operator[oper]);
					}

					isStringComparing = left_tp->kind == TP_int?FALSE:TRUE;
					te = TL_logicExp(oper, left_exp_tp.exp, right_exp_tp.exp, isStringComparing);
					return Exp_Tp(te, TP_Int());
				}
				break;
		case AST_seqExp:
			// calculate the number of exps in the list
			i = 0;
			for(expList = e->u.seq;expList !=NULL; expList = expList->tail){
				if(expList->head != NULL){
					i ++;
				}
			}
			// if expList has no expression, return empty
			if (i == 0){
				exp_tp = Exp_Tp(TL_voidExp(), TP_Void());
			}
			else{
				// create an array to store each sub expression
				te_list = check_malloc(i * sizeof(*te_list));
				
				// generate IR for each sub exp
				for(expList = e->u.seq, i = 0;expList != NULL; expList = expList->tail){
					exp = expList->head;
					if (exp != NULL) {
						exp_tp = transExp(level, valueEV, typeEV, exp);

						te_list[i] = exp_tp.exp;

						i++;
					}
				}
				exp_tp = Exp_Tp(TL_seqExp(te_list, i), exp_tp.tp);
				check_free(te_list);

			}
			return exp_tp;
		case AST_assignExp:
			/* translate the assignee.
			 * if the var is not defined, an error would occur in transVar()
			 */
			var_exp = transVar(level, valueEV, typeEV, e->u.assign.var);

			// translate the assigner
			exp_tp = transExp(level, valueEV, typeEV, e->u.assign.exp);

			// check whether the assignee has a type incompatible with the assigner
			tp = actual_tp(var_exp.tp);
			if(tp !=NULL && !(looseTypeCompare(tp, exp_tp.tp))){
				SM_info(ERROR, e->u.assign.var->pos, 
						"variable is assigned with a value whose type is incompatible with the declared one!");
				return Exp_Tp(TL_assignExp(var_exp.exp, NULL), TP_Void());
			}
			SM_check_exit(SEVERE);

			return Exp_Tp(TL_assignExp(var_exp.exp, exp_tp.exp), TP_Void());
		case AST_ifExp:
			// translate compare exp
			compare_exp = transExp(level, valueEV, typeEV, e->u.iff.test);
			if(actual_tp(compare_exp.tp)->kind != TP_int){
				SM_info(FATAL, e->u.iff.test->pos, "the condition test of if expression must be of integer type!");
			}

			// translate the true-branch exp
			true_exp = transExp(level, valueEV, typeEV, e->u.iff.then);

			// translate the false-branch exp
			if(e->u.iff.elsee != NULL){
				is_if_then_else = TRUE;
				false_exp = transExp(level, valueEV, typeEV, e->u.iff.elsee);
			}
			else{
				is_if_then_else = FALSE;
			}

			// compare type compatibility
			if(!is_if_then_else){
				if(true_exp.tp->kind != TP_void){
					SM_info(WARNING, e->u.iff.then->pos, "value returned from if-then expression is ignored!");
				}
				exp_tp = Exp_Tp(TL_ifExp(compare_exp.exp, true_exp.exp, NULL), TP_Void());
			}
			else{
				if(!looseTypeCompare(true_exp.tp, false_exp.tp)){
					SM_info(FATAL, e->u.iff.then->pos, "value returned from THEN branch must share the same type of that from ELSE branch!");
				}
				exp_tp = Exp_Tp(TL_ifExp(compare_exp.exp, true_exp.exp, false_exp.exp), true_exp.tp);
			}
			return exp_tp;
		case AST_whileExp:
			/*
			 * add a empty loop variant to loop_var_stack
			 */
			Loop_var_stack_push(NULL);

			/*
			 * generate a new done label
			 */
			TL_genLoopDoneLabel();

			// translate test exp
			compare_exp = transExp(level, valueEV, typeEV, e->u.whilee.test);
			if(actual_tp(compare_exp.tp)->kind != TP_int){
				SM_info(FATAL, e->u.whilee.test->pos, "the condition of while expression must be of integer type!");
			}

			// translate the loop body
			body_exp = transExp(level, valueEV, typeEV, e->u.whilee.body);

			if(actual_tp(body_exp.tp)->kind != TP_void){
				SM_info(WARNING, e->pos, "Value returned from WHILE expression is ignored.");
			}

			// pop the exmpty loop variant
			Loop_var_stack_pop();

			return Exp_Tp(TL_whileExp(compare_exp.exp, body_exp.exp), TP_Void());
		case AST_forExp:
			// enter a new scope
			SB_beginScope(valueEV);

			// check whether the variant's name has been used as loop variant in outer loops
			var_sym = e->u.forr.var;
			if(Loop_var_stack_look(var_sym)){
				SM_info(SEVERE, e->pos, "the name '%s' has been used by outer loop's variant", SB_name(var_sym));
			}	
			else{
				Loop_var_stack_push(var_sym);
			}

			// generate a new done label
			TL_genLoopDoneLabel();

			// translate the start and end
			for_start_exp = transExp(level, valueEV, typeEV, e->u.forr.lo);
			for_end_exp = transExp(level, valueEV, typeEV, e->u.forr.hi);
	
			// check the legality of start and end
			if(actual_tp(for_start_exp.tp)->kind != TP_int){
				SM_info(SEVERE, e->u.forr.lo->pos, "the start bound of for expression must be of integer type!");
			}
			if(actual_tp(for_end_exp.tp)->kind != TP_int){
				SM_info(SEVERE, e->u.forr.lo->pos, "the end bound of for expression must be of integer type!");
			}

			// enter the variable into env
			tl_access = TL_allocLocal(level, TRUE);
			SB_enter(valueEV, var_sym, EV_VarItem(tl_access, TP_Int()));
			var_exp = transVar(level, valueEV, typeEV, AST_SimpleVar(e->pos, var_sym));

			// translate loop body
			body_exp = transExp(level, valueEV, typeEV, e->u.forr.body);
			if(actual_tp(body_exp.tp)->kind != TP_void){
				SM_info(WARNING, e->pos, "value returned from while expression is ignored.");
			}

			SM_check_exit(SEVERE);

			Loop_var_stack_pop();
			SB_endScope(valueEV);

			return Exp_Tp(TL_forExp(var_exp.exp, for_start_exp.exp, for_end_exp.exp, body_exp.exp), TP_Void());
		case AST_letExp:
			// begin new scope
			SB_beginScope(valueEV);
			SB_beginScope(typeEV);

			// calculate num of declarations between let and in
			i = 0;
			for(d = e->u.let.decs; d; d = d->tail){
				if(d->head!=NULL){
					i ++;
				}
			}

			// create an array to store all the expressions, including 
			// variable declaration
			// seqExp between in and end
			te_list = check_malloc((i + 1) * sizeof(*te_list));
			i = 0;

			// translate declarations in given order
			for(d = e->u.let.decs;d;d = d->tail){
				if(d->head != NULL){
					te_list[i] = transDec(level, valueEV, typeEV, d->head);
					i ++;
				}
			}

			// translate body between in and end
			exp_tp = transExp(level, valueEV, typeEV, e->u.let.body);

			te_list[i] = exp_tp.exp;
				
			// generate a IR tree putting together
			exp_tp.exp = TL_seqExp(te_list, i + 1);
			check_free(te_list);

			// exit scope
			SB_endScope(typeEV);
			SB_endScope(valueEV);

			return exp_tp;
		case AST_arrayExp:
			// check whether symbol is an array type
			tp = SB_look(typeEV, e->u.array.typ);
			if(tp!= NULL){
				tp = actual_tp(tp);
				if(tp->kind != TP_array){
					SM_info(SEVERE, e->pos, "Type '%s' is not array", SB_name(e->u.array.typ));
				}
			}
			else{
				SM_info(FATAL, e->pos, "array type '%s' doesn't exist!", SB_name(e->u.array.typ));
			}

			// check whether the size is integer type
			exp_tp = transExp(level, valueEV, typeEV, e->u.array.size);
			if(actual_tp(exp_tp.tp)->kind != TP_int){
				SM_info(SEVERE, e->u.array.size->pos, "size of an array must be of integer type!");
			}

			// check whether the initializer has the same type as that of array's element type
			arr_tp = actual_tp(tp->u.array);
			arr_exp_tp = transExp(level, valueEV, typeEV, e->u.array.init);

			if(arr_tp != actual_tp(arr_exp_tp.tp)){
				SM_info(SEVERE, e->u.array.init->pos, "initializer of an array's element must follow declared type!");
			}

			SM_check_exit(SEVERE);

			return Exp_Tp(TL_arrayExp(exp_tp.exp, arr_exp_tp.exp), tp);
		default:
			printf("Illegal AST Node!\n");
			exit(2);
	}
	exit(2);
}

struct exp_tp transVar(TL_level level, SB_table valueEV, SB_table typeEV, AST_var v){
	EV_item item;

	struct exp_tp exp;
	struct exp_tp exp2;

	TL_exp te;
	int offset;

	switch(v->kind){
		case AST_simpleVar:
			item = SB_look(valueEV, v->u.simple);
			if(item != NULL && item->kind == EV_varItem){
				te = TL_simpleVar(item->u.var.access, level);
				return Exp_Tp(te, actual_tp(item->u.var.tp));
			}
			else{
				SM_info(FATAL, v->pos, "undefined variable: '%s'", SB_name(v->u.simple));
				break;
			}
		case AST_subscriptVar:
			// translate the part ahead of '[', which must be an array
			exp = transVar(level, valueEV, typeEV, v->u.subscript.var);
			if(exp.tp->kind != TP_array){
				SM_info(FATAL, v->pos, "index access to a non-array variable!");
				break;
			}

			// translate the part between [ and ], which should be an int
			exp2 = transExp(level, valueEV, typeEV, v->u.subscript.exp);
			if(exp2.tp->kind != TP_int){
				SM_info(FATAL, v->pos, "index access to array variable must be of integer type!");
				break;
			}
			te = TL_arrayVar(exp.exp, exp2.exp);

			return Exp_Tp(te, actual_tp(exp.tp->u.array));
	}
	return SM_check_exit(FATAL);
}
TL_exp transDec(TL_level level, SB_table valueEV, SB_table typeEV, AST_dec d){
	SB_symbol sym;
	AST_nametyList types;
	AST_namety type;
	struct exp_tp exp;
	TP_tp tp;
	TP_tp decTp;
	TL_access access;
	TL_exp tl_exp;
	AST_fundecList fundecList;
	AST_fundec head;
	TP_tp resultTp;
	TP_tpList formalTps;
	UN_boolList formal_escapes;
	TL_level funcLevel;
	EV_item item;
	TL_accessList tl_formals;
	AST_fieldList fieldList;
	TP_tpList tpList;
	switch(d->kind){
		case AST_varDec:
			sym = d->u.var.var;

			// check whether the variable has been used as loop variable
			if(Loop_var_stack_look(sym)){
				SM_info(FATAL, d->pos, "the name of loop variable '%s' can not be reused!", SB_name(sym));
			}

			// translate the initializer
			exp = transExp(level, valueEV, typeEV, d->u.var.init);
			tp = actual_tp(exp.tp);

			// determine the var's type
			if(d->u.var.typ != NULL){
				decTp = SB_look(typeEV, d->u.var.typ);
				if(decTp == NULL || !looseTypeCompare(tp, decTp)){
					SM_info(ERROR, d->pos, "The initializer of variable '%s' is incompatible with the declared type!", SB_name(sym));
				}	
			}
			else {
				decTp = exp.tp;
			}
			
			// enter the var into the env
			access = TL_allocLocal(level, TRUE);
			SB_enter(valueEV, sym, EV_VarItem(access, decTp));

			tl_exp = TL_assignExp(transVar(level, valueEV, typeEV, AST_SimpleVar(d->pos, d->u.var.var)).exp, exp.exp);

			break;
		case AST_typeDec:
			Type_sym_stack_empty();

			
			for(types = d->u.type;types != NULL;types = types->tail){
				type = types->head;
				if(type != NULL){
					// check whether it has been used in this block
					if(Type_sym_stack_look(type->name)){
						SM_info(FATAL, d->pos, "the type name '%s' has been used adjacently!", SB_name(type->name));
					}
					else{
						Type_sym_stack_push(type->name);
					}

					SB_enter(typeEV, type->name, generateTp(type->ty));
				}

			}

			// translate type declarations
			for(types = d->u.type; types != NULL; types = types->tail){
				type = types->head;
				if(type != NULL){
					transTp(typeEV, type->name, type->ty);
				}
			}

			tl_exp = TL_voidExp();
			break;
		case AST_functionDec:
			// pusuh formals and result of function declarations into the environment
			// alse check whether there are functions of the same name
			
			Type_sym_stack_empty();

			for (fundecList = d->u.function; fundecList != NULL; fundecList = fundecList->tail) {
				head = fundecList->head;
				if (head != NULL) {

					//check whether it has been used 
					if (Type_sym_stack_look(head->name)) {
						SM_info(FATAL, d->pos, "the function name '%s' has been used!", SB_name(head->name));
					}
					else {
						Type_sym_stack_push(head->name);
					}

					if (head->result != NULL) {
						resultTp = SB_look(typeEV, head->result);
					}
					else {
						resultTp = TP_Void();
					}

					formalTps = generateFormalTpList(typeEV, head->params);

					generateEscapeList(&formal_escapes, head->params);
					
					funcLevel = TL_newLevel(level, TMP_namedlabel(SB_name(head->name)), formal_escapes);
					check_free(formal_escapes);

					SB_enter(valueEV, head->name, EV_FunItem(funcLevel, TMP_newlabel(), formalTps, resultTp));


				}
			}

			for (fundecList = d->u.function; fundecList != NULL; fundecList = fundecList->tail) {
				head = fundecList->head;
				if (head != NULL) {

					// get formaltypes
					item = SB_look(valueEV, head->name);
					formalTps = item->u.fun.formalTypes;

					SB_beginScope(valueEV);
					tl_formals = TL_formals(item->u.fun.level)->tail;
					for (fieldList = head->params, tpList = formalTps; fieldList != NULL;
						fieldList = fieldList->tail, tpList = tpList->tail, tl_formals = tl_formals->tail) {
						SB_enter(valueEV, fieldList->head->name, EV_VarItem(tl_formals->head, tpList->head));
					}

					exp = transExp(item->u.fun.level, valueEV, typeEV, head->body);

					resultTp = item->u.fun.resultType;
					if (head->result != NULL) {
						tp = actual_tp(exp.tp);
						if (actual_tp(resultTp) != tp) {
							if (tp->kind == TP_void) {
								SM_info(ERROR, head->pos, "the function '%s' has no returned value!", SB_name(head->name));
							}
							else {
								SM_info(ERROR, head->pos, "the returned value of function '%s' is incompatible with declared type", SB_name(head->name));
							}
						}
					}
					else {
						if (actual_tp(exp.tp) != TP_void) {
							SM_info(WARNING, head->pos, "returned value of procedure '%s' is ignored!", SB_name(head->name));
						}
					}

					TL_procEntryExit(item->u.fun.level, exp.exp, TL_formals(item->u.fun.level), item->u.fun.label);

					SB_endScope(valueEV);
				}
			}

			tl_exp = TL_voidExp();
			break;
	}
	return tl_exp;
}

struct exp_tp Exp_Tp(TL_exp exp, TP_tp tp){
	struct exp_tp et;
	et.exp = exp;
	et.tp = tp;
	return et;
}

/*
 * ENV module
 */
EV_item EV_VarItem(TL_access access, TP_tp tp){
	EV_item item = check_malloc(sizeof(*item));
	item->kind = EV_varItem;
	item->u.var.tp = tp;
	item->u.var.access = access;
	return item;
}

EV_item EV_FunItem(TL_level level, TMP_label label, TP_tpList formalTypes, TP_tp resultType) {
	EV_item item = check_malloc(sizeof(*item));
	item->kind = EV_funItem;
	item->u.fun.formalTypes = formalTypes;
	item->u.fun.label = label;
	item->u.fun.level = level;
	item->u.fun.resultType = resultType;
	return item;
}

SB_table EV_base_typeEv(){
	SB_table table = SB_empty();
	SB_enter(table, SB_Symbol("int"), TP_Int());
	SB_enter(table, SB_Symbol("string"), TP_String());
	return table;
}

SB_table EV_base_valueEv(){
	SB_table table = SB_empty();
	TL_level level;

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("print"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("print"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), NULL), TP_Void()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("flush"), NULL);
	SB_enter(table, SB_Symbol("flush"), EV_FunItem(level, TMP_newlabel(), NULL, TP_Void()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("getchar"), NULL);
	SB_enter(table, SB_Symbol("getchar"), EV_FunItem(level, TMP_newlabel(), NULL, TP_String()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("ord"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("ord"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), NULL), TP_Int()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("chr"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("chr"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_Int(), NULL), TP_String()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("size"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("size"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), NULL), TP_Int()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("substring"), UN_BoolList(TRUE, UN_BoolList(TRUE, UN_BoolList(TRUE, NULL))));
	SB_enter(table, SB_Symbol("substring"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), TP_TpList(TP_Int(), TP_TpList(TP_Int(), NULL))), TP_String()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("concat"), UN_BoolList(TRUE, UN_BoolList(TRUE, NULL)));
	SB_enter(table, SB_Symbol("concat"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), TP_TpList(TP_String(), NULL)), TP_String()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("not"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("not"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_String(), NULL), TP_Int()));

	level = TL_newLevel(TL_outermost(), TMP_namedlabel("exit"), UN_BoolList(TRUE, NULL));
	SB_enter(table, SB_Symbol("exit"), EV_FunItem(level, TMP_newlabel(), TP_TpList(TP_Int(), NULL), TP_Void()));
	return table;
}

static TP_tp actual_tp(TP_tp tp){
	return tp;
}

