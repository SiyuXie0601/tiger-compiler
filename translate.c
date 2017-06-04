/*********************************************************************************
*FileName: translate.c
*Author: Debbie Xie
*Create Time: 2017/5/25
*Description: TIGER'S IR Translation Module
**********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "frame.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "mipsframe.h"
#include "absyn.h"
#include "translate.h"
#include "printtree.h"



/* Stack Structure START */

/* Push */
void Stack_push(stack_item *stkList, void *key){
	stack_item head = *stkList;
	if(head == NULL){
		head = check_malloc(sizeof(struct stack_item_));
		head->key = key;
		head->next = NULL;
	}
	else{
		stack_item item = check_malloc(sizeof(struct stack_item_));
		item->key = key;
		item->next = head;
		head = item;
	}
	*stkList = head;
}

/*Pop*/
void Stack_pop(stack_item *stkList){
	stack_item head = *stkList;
	if(head != NULL){
		stack_item free = head;
		head = head->next;
		check_free(free);
	}
	*stkList = head;
}

/*Make the stack empty*/
void Stack_empty(stack_item *stkList){
	stack_item head = *stkList;
	stack_item empty;
	while(head != NULL){
		empty = head;
		head = head->next;
		check_free(empty);
	}
	*stkList = head;
}

/*Check whether the given key exists in the stack.
 *Function compare return true if two variables are the same.
 */
bool Stack_check(stack_item *stkList, void *key, bool (*compare)(void*, void*)){
	stack_item ptr = *stkList;
	while(ptr != NULL){
		if(compare(ptr->key,key)){
			return TRUE;
		}
		ptr = ptr->next;
	}
	return FALSE;
}

/*return key of the first item*/
void* Stack_peek(stack_item *stkList){
	return (*stkList)->key;
}
/* Stack Structure END */

/*Loop Label Stack
 *Since IR Tree is constructed by recursion
 *We need to produce done label and store it at the first time
 *so that when break statement is constructed
 *it knows where to jump to
 */
static stack_item L_label_stack = NULL;

static void LLS_push(TMP_label label){
	Stack_push(&L_label_stack, label);
}

static void LLS_pop(){
	Stack_pop(&L_label_stack);
}

static TMP_label LLS_peek(){
	return Stack_peek(&L_label_stack);
}
/*Loop Label Stack end*/

/*??????????????????????????????????????????????*/
TL_expList TL_ExpList(TL_exp head, TL_expList tail){

}

/*??????????????????????????????????????????????*/
TL_accessList TL_AccessList(TL_access head, TL_accessList tail);

struct TL_access_{
	TL_level level;
	FRM_access access;
};

struct TL_level_{
	TL_level parent;
	FRM_frame frame;
	int depth;
};

static TL_level outermostLevel = NULL;

TL_level TL_outermost(void){
	if(outermostLevel == NULL){
		outermostLevel = check_malloc(sizeof(struct TL_level_));
		outermostLevel->parent = NULL;
		outermostLevel->frame = FRM_newFrame(TMP_namedlabel("main"),NULL);
		outermostLevel->depth = 0;
	}
	return outermostLevel;
}

/*Related to function.
 *Will be implemented in second version.
 */
TL_level TL_newLevel(TL_level parentb, TMP_label nameb, UN_boolList formalsb){
	TL_level levelb = (TL_level)check_malloc(sizeof(struct TL_level_));
	levelb->parent = parentb;
	levelb->frame = FRM_newFrame(nameb, UN_BoolList(TRUE, formalsb));//Add a new formal used as static link
	levelb->depth = parentb->depth + 1;
	return levelb;
}
TL_accessList TL_formals(TL_level levelb){
	FRM_accessList frmAccList = FRM_formals(levelb->frame);
	TL_accessList accListb = NULL;
	TL_accessList accListHead = NULL;

	for(;frmAccList != NULL; frmAccList = frmAccList->tail){
		if(accListHead == NULL){
			accListb = (TL_accessList)check_malloc(sizeof(struct TL_accessList_));
			accListHead = accListb;
		}
		else{
			accListb->tail = (TL_accessList)check_malloc(sizeof(struct TL_accessList_));
			accListb = accListb->tail;
		}
		accListb->head = (TL_access)check_malloc(sizeof(struct TL_access_));
		accListb->head->level = levelb;
		accListb->head->access = frmAccList->head;
	}
	if(accListb != NULL){
		accListb->tail = NULL;
	}
	return accListHead;
}

TL_level TL_getParent(TL_level parent){

}

void TL_printLevel(TL_level level){

}

TL_access TL_allocLocal(TL_level level, bool escape){
	FRM_access access = FRM_allocLocal(level->frame, escape);
	TL_access tlacc = check_malloc(sizeof(struct TL_access_));

	tlacc->access = access;
	tlacc->level = level;

	return tlacc;
}

/*construct a new patchlist based on the old patchlist
 *Here arg label is the address on the IR tree*/
static patchList PatchList(TMP_label *label, patchList list){
	patchList new = check_malloc(sizeof(struct patchList_));
	new->labelOnTree = label;
	new->next = list;
	return new;
}

static void doPatch(patchList plist, TMP_label label){
	while(plist != NULL){
		if(*plist->labelOnTree == NULL)
			*plist->labelOnTree = label;
		plist = plist->next;
	}
}

/*?????????????????????????????????????????*/
void TL_printTrExp(TL_exp te);
void TL_printResult();
void TL_printTree(TL_exp exp){
	printStmList(stdout, TR_StmList(unNx(exp),NULL));
}

/*Constructors*/
static TL_exp TL_Ex(TR_exp ex){
	TL_exp exp = check_malloc(sizeof(struct TL_exp_));
	exp->kind = TL_ex;
	exp->u.ex = ex;
	return exp;
}

static TL_exp TL_Nx(TR_stm nx){
	TL_exp exp = check_malloc(sizeof(struct TL_exp_));
	exp->kind = TL_nx;
	exp->u.nx = nx;
	return exp;
}

static TL_exp TL_Cx(patchList trueList, patchList falseList, TR_stm stm){
	TL_exp exp = check_malloc(sizeof(struct TL_exp_));
	exp->kind = TL_cx;
	exp->u.cx.trues = trueList;
	exp->u.cx.falses = falseList;
	exp->u.cx.stm = stm;
}

/*Extractors
 *in tree.h and tree.c
 functions are named by TR_ + some parts' name which will be translated into statement or expression later'*/
static TR_exp unEx(TL_exp exp){
	TR_exp tree;
	TMP_temp value;
	TMP_label tLabel, fLabel;

	switch(exp->kind){
		case TL_ex:
			tree = exp->u.ex;
			break;
		case TL_nx:
			tree = TR_Eseq(exp->u.nx, TR_Const(0));
			break;
		case TL_cx:
			value = TMP_newtemp();
			tLabel = TMP_newlabel();
			fLabel = TMP_newlabel();
			doPatch(exp->u.cx.trues, tLabel);
			doPatch(exp->u.cx.falses, fLabel);
			tree = TR_Eseq(TR_Move(TR_Temp(value), TR_Const(1)),
						TR_Eseq(exp->u.cx.stm,
							TR_Eseq(TR_Label(fLabel),
								TR_Eseq(TR_Move(TR_Temp(value), TR_Const(0)),
									TR_Eseq(TR_Label(tLabel),
										    TR_Temp(value)
									)
								)
							)
						)
				   );
			break;
	}
	return tree;
}

static TR_stm unNx(TL_exp exp){
	TR_stm tree;
	TMP_label tLabel, fLabel;
	switch(exp->kind){
		case TL_ex:
			tree = TR_Exp(exp->u.ex);
			break;
		case TL_nx:
			tree = exp->u.nx;
			break;
		case TL_cx:
			tLabel = TMP_newlabel();
			fLabel = TMP_newlabel();
			doPatch(exp->u.cx.trues, tLabel);
			doPatch(exp->u.cx.falses, fLabel);
			tree = TR_Seq(exp->u.cx.stm, TR_Seq(TR_Label(tLabel), TR_Label(fLabel)));
			break;
	}
	return tree;
}

/*An example of this situation:
 * if(a + 1 > b){}
 */
static struct Cx unCx(TL_exp exp){
	 struct Cx conditionx;

	 switch(exp->kind){
		 case TL_ex:
			 conditionx.stm = TR_Cjump(TR_ne, exp->u.ex, TR_Const(0),NULL, NULL);
			 conditionx.trues = PatchList(&conditionx.stm->u.CJUMP.true, NULL);
			 conditionx.falses = PatchList(&conditionx.stm->u.CJUMP.false, NULL);
			 break;
		 case TL_cx:
			 conditionx = exp->u.cx;
			 break;
		 case TL_nx:
			 conditionx.trues = NULL;
			 conditionx.falses = NULL;
			 conditionx.stm = exp->u.nx;
			 break;
	 }
	 return conditionx;
}

//Tranlation for expressions
TL_exp TL_nilExp();
TL_exp TL_assignExp(TL_exp lval, TL_exp rval){
	if(rval != NULL){
		return TL_Nx(TR_Move(unEx(lval),unEx(rval)));
	}
	else{
		return lval;
	}
}
TL_exp TL_forExp(TL_exp vari, TL_exp lowv, TL_exp highv, TL_exp body){
	TR_stm statement;
	TMP_label tLabel = TMP_newlabel();
	TMP_label fLabel = LLS_peek();//Get donw label from the list
    TMP_label sLabel = TMP_newlabel();

	TR_exp variable = unEx(vari);
	TR_stm condition = TR_Cjump(TR_le, variable, unEx(highv), tLabel, fLabel );
	statement = TR_Seq(TR_Move(variable,unEx(lowv)),
					 TR_Seq(TR_Label(sLabel),
						 TR_Seq(condition,
							 TR_Seq(TR_Label(tLabel),
								 TR_Seq(unNx(body),
									 TR_Seq(TR_Move(variable, TR_Binop(TR_plus, variable, TR_Const(1))),
										 TR_Seq(TR_Jump(TR_Name(sLabel),TMP_LabelList(sLabel,NULL)),
											    TR_Label(fLabel)
											   )
										   )
								       )
								   )   
							   )
						   )
					  );
	LLS_pop();
	return TL_Nx(statement);

}
TL_exp TL_whileExp(TL_exp condition, TL_exp body){

	TR_stm statement;
	struct Cx cx_condition = unCx(condition);

	TMP_label tLabel = TMP_newlabel();
	TMP_label fLabel = LLS_peek();//Get done label from the list
	TMP_label sLabel = TMP_newlabel();
	doPatch(cx_condition.trues, tLabel);
	doPatch(cx_condition.falses, fLabel);

	statement = TR_Seq(TR_Label(sLabel),
					TR_Seq(cx_condition.stm,
						TR_Seq(TR_Label(tLabel),
							TR_Seq(unNx(body),
								TR_Seq(TR_Jump(TR_Name(sLabel),TMP_LabelList(sLabel, NULL)),
									TR_Label(fLabel)
								)
							)
						)
					)
		     	);
	LLS_pop();
	return TL_Nx(statement);
	
}

void TL_genLoopDoneLabel(){
	TMP_label fLabel = TMP_newlabel();
	LLS_push(fLabel);
}

TL_exp TL_breakExp();
TL_exp TL_ifExp(TL_exp condition, TL_exp thenc, TL_exp elsec){

	struct Cx conditionx = unCx(condition);
	TMP_label tLabel = TMP_newlabel();
	TMP_label fLabel = TMP_newlabel();
	TMP_label jLabel = TMP_newlabel();

	doPatch(conditionx.trues, tLabel);
	doPatch(conditionx.falses, fLabel);


    /*if-then*/
	if(elsec == NULL){
		return TL_Nx(TR_Seq(conditionx.stm,
						TR_Seq(TR_Label(tLabel),
							TR_Seq(unNx(thenc),
								   TR_Label(fLabel)
							      )
							)
					)
			   );
	}
	/*if-then-else*/
	/*if this statement doesn't have return value*/
	else{
		if(thenc->kind == TL_nx && elsec->kind == TL_nx){
			return TL_Nx(TR_Seq(conditionx.stm,
							TR_Seq(TR_Label(tLabel),
								TR_Seq(unNx(thenc),
									TR_Seq(TR_Jump(TR_Name(jLabel),TMP_LabelList(jLabel,NULL)),
										TR_Seq(TR_Label(fLabel),
											TR_Seq(unNx(elsec),
												   TR_Label(jLabel)
												  )
											  )
										  )
									  )
								  )
						       )
					    );
		}
		/*if this statement has return value*/
		else{
			TMP_temp rLabel = TMP_newtemp();
			return TL_Ex(TR_Eseq(conditionx.stm,
							TR_Eseq(TR_Label(tLabel),
								TR_Eseq(TR_Move(TR_Temp(rLabel),unEx(thenc)),
									TR_Eseq(TR_Jump(TR_Name(jLabel),TMP_LabelList(jLabel, NULL)),
										TR_Eseq(TR_Label(fLabel),
											TR_Eseq(TR_Move(TR_Temp(rLabel),unEx(elsec)),
												TR_Eseq(TR_Label(jLabel),TR_Temp(rLabel))  
											   )
										   )
									   )
								   )
				                )
					    )
				   );
		}
	}
	
}
TL_exp TL_simpleVar(TL_access translate_acc, TL_level translate_level){
	TR_exp expression;
	if(translate_level != translate_acc->level){
		/*Follow static link
		 *offset(0) of each frame stores the frame ptr of callee function*/
		expression = FRM_Exp(FRM_staticLink(),TR_Temp(FRM_FP()));
		translate_level = translate_level->parent;
		while(translate_level != translate_acc->level){
			expression = FRM_Exp(FRM_staticLink(),expression);
			translate_level = translate_level->parent;
		}
		expression = FRM_Exp(translate_acc->access, expression);
	}
	else{
		expression = FRM_Exp(translate_acc->access, TR_Temp(FRM_FP()));
	}
	return TL_Ex(expression);
}

/*TL_arrayVar return IR representation of an element's address in an array
 *var stores array's head element's position */
TL_exp TL_arrayVar(TL_exp var, TL_exp offset_exp){
	return TL_Ex(TR_Mem(TR_Binop(TR_plus, unEx(var),TR_Binop(TR_mul, unEx(offset_exp),TR_Const(FRM_wordSize)))));	
}

TL_exp TL_intExp(int int_variable){
	return TL_Ex(TR_Const(int_variable));
}

TL_exp TL_stringExp(string str_variable){
	TMP_label stringLabel = TMP_newlabel();
	/*add a string label in fragList head*/
	FRM_String(stringLabel, str_variable);
	return TL_Ex(TR_Name(stringLabel));
}

TL_exp TL_arithExp(AST_oper opera, TL_exp leftExp, TL_exp rightExp){
	TR_exp expression;

	switch(opera){
		case AST_plusOp:
			expression = TR_Binop(TR_plus, unEx(leftExp), unEx(rightExp));
			break;
		case AST_minusOp:
			expression = TR_Binop(TR_minus, unEx(leftExp),unEx(rightExp));	
			break;
		case AST_timesOp:
			expression = TR_Binop(TR_mul, unEx(leftExp), unEx(rightExp));
			break;
		case AST_divideOp:
			expression = TR_Binop(TR_div, unEx(leftExp),unEx(rightExp));
			break;
		default:
			/*It shouldn't been reached'*/
			printf("Illegal arithmetic operator reach here!");
			exit(2);
	}

	return TL_Ex(expression);
}

TL_exp TL_logicExp(AST_oper opera, TL_exp leftExp, TL_exp rightExp, bool strCompare){
	TR_stm statement;
	patchList tLabel;
	patchList fLabel;

	if(strCompare){
		TR_exp stringCompareCall = FRM_externalCall("stringEqual", TR_ExpList(unEx(leftExp), TR_ExpList(unEx(rightExp),NULL)));
		switch(opera){
			case AST_eqOp:
				statement = TR_Cjump(TR_eq, stringCompareCall, TR_Const(1), NULL, NULL);
				break;
			case AST_neqOp:
				statement = TR_Cjump(TR_eq, stringCompareCall, TR_Const(0), NULL, NULL);
				break;
			default:
				/*It shouldn't been reached'*/
				printf("Illegal logical operator in string compare reach here");
				exit(2);
		}
	}
	else{
		switch(opera){
			case AST_eqOp:
				statement = TR_Cjump(TR_eq, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			case AST_neqOp:
				statement = TR_Cjump(TR_ne, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			case AST_ltOp:
				statement = TR_Cjump(TR_lt, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			case AST_gtOp:
				statement = TR_Cjump(TR_gt, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			case AST_leOp:
				statement = TR_Cjump(TR_le, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			case AST_geOp:
				statement = TR_Cjump(TR_ge, unEx(leftExp), unEx(rightExp), NULL, NULL);
				break;
			default:
				printf("Illegal logical operator reach here");
				exit(2);		
		}
	}
	tLabel = PatchList(&statement->u.CJUMP.true, NULL);
	fLabel = PatchList(&statement->u.CJUMP.false, NULL);
	return TL_Cx(tLabel, fLabel, statement);

}

/*Init array */
/*here I change Ex to Nx*/
TL_exp TL_arrayExp(TL_exp sizeOfArray, TL_exp initVal){
	return TL_Ex(FRM_externalCall("initArray", TR_ExpList(unEx(sizeOfArray), TR_ExpList(unEx(initVal), NULL))));
}

TL_exp TL_seqExp(TL_exp* arrayOfStm, int sizeOfArray){
	TR_exp* p = check_malloc(sizeof(TR_exp));
	TR_exp head;

	int cnt = 0;
	int last = sizeOfArray - 1;
	while(cnt < sizeOfArray){
		if(cnt != last){
			*p = TR_Eseq(unNx(arrayOfStm[cnt]), NULL);
			if (cnt == 0) {
				head = *p;
			}
			p = &((*p)->u.ESEQ.exp);
		}
		else{
			*p = unEx(arrayOfStm[cnt]);
			if (cnt == 0) {
				head = *p;
			}
		}
		cnt++;
	}
	return TL_Ex(head);
}

static TL_exp TL_voidReturn = NULL;
TL_exp TL_voidExp(void){
	if(TL_voidReturn == NULL){
		TL_voidReturn = TL_Ex(TR_Const(0));
	}
	return TL_voidReturn;
}

TL_exp TL_callExp(TL_level caller_lvl, TL_level callee_lvl, TMP_label fun_label, TL_exp* argv, int args);

void TL_procEntryExit(TL_level funLevel, TL_exp funBody, TL_accessList formals, TMP_label label){
	TR_stm statement;
	statement = TR_Seq(
					TR_Label(label),
					TR_Move(TR_Temp(FRM_RV()), unEx(funBody))
	            );
	/*In fragList stores frag about string and function
	 *Now put the function frag into fragList*/
	FRM_Proc(statement, funLevel->frame);
}

//Fragment list
FRM_fragList TL_getResult(void){
	return FRM_getFragList();
}

