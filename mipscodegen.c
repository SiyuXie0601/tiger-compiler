/*********************************************************************************
*FileName: mipscodegen.c
*Author: Debbie Xie
*Create Time: 2017/6/14
*Description: TIGER'S Assembly Code Generation
**********************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "temp.h"
#include "assem.h"
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "mipsframe.h"
#include "errormsg.h"
#include "codegen.h"


static ASSB_instrList FRM_procEntryExit2(ASSB_instrList body);
static TMP_temp munchExpression(TR_exp exp);
static void munchStatement(TR_stm stm);
static void codeEmit(ASSB_instr inst);
static TMP_tempList TMPList(TMP_temp h, TMP_tempList t);
static ASSB_instrList instructionList = NULL, instructionLast = NULL;

static TMP_tempList munchArguments(TR_expList arguments, int *number);
static TMP_tempList getCalldefs();
static TMP_temp codeEmitBinop(char *operand, TR_exp expression0, TR_exp expression1);
static TMP_temp codeEmitShiftop(char *operand, TR_exp expression0, TR_exp expression1);
static void codeEmitCondJump(char *operand, TR_exp treeLeft, TR_exp treeRight, TMP_label trueLabel, TMP_label falseLabel);
static void codeEmitComment(char *msg);

static char* build(char *msg, ...);
static void codeError(char *msg);

extern const int FRM_wordSize;
static FRM_frame thisFRM;
static TMP_temp *callerSaveRegArray = NULL;
static int callerSaveRegsNumber = -1;
static void callerRestoreRegs();
static void callerSaveRegs();
static inline void initCallerSaveRegs();

static char* build(char *msg, ...){
    va_list va_ap;
    va_start(va_ap, msg);

    char *h = check_malloc(80 * sizeof(char));
    char *p = h;
    int i = vsprintf(p, msg, va_ap);
    p = p + i;
    sprintf(p, "\n\0");
    va_end(va_ap);

    return h;
}

static void codeError(char *msg){
    printf("[Assemble Codegen Error] %s\n", msg);
    exit(1);
}

static TMP_tempList munchArguments(TR_expList arguments, int * number){
    int cnt = 0;
    if(arguments == NULL){
        return NULL;
    }
    TR_exp argument = NULL;
    TMP_tempList t_list = NULL;
    TMP_tempList t_listHead = NULL;

    for(;arguments !=NULL; arguments = arguments->tail){
        argument = arguments->head;
        if(argument != NULL){
            if(t_list == NULL){
                t_list = TMP_TempList(munchExpression(argument), NULL);
                t_listHead = t_list;
            }
            else{
                t_list->tail = TMP_TempList(munchExpression(argument),NULL);
                t_list = t_list->tail;
            }
        }
        cnt++;
    }
    *number = cnt;
    return t_listHead;
}

static TMP_tempList calldefines = NULL;

static TMP_tempList getCalldefs(){
    if(getCalldefs == NULL){
        TMP_tempList t_list = FRM_getRegList(specialRegisters);
        calldefines = t_list;
        while(t_list->tail != NULL){
            t_list = t_list->tail;
        }
        t_list->tail = FRM_getRegList(callersaveRegisters);
    }
    return calldefines;
}

static TMP_temp codeEmitBinop(char *operand, TR_exp expression0, TR_exp expression1){
    TMP_temp r_label = TMP_newtemp();
    codeEmit(ASSB_Oper(
        build("%s `d0, `s0, `s1", operand),
        TMPList(r_label, NULL),
        TMPList(munchExpression(expression0),
            TMPList(munchExpression(expression1), NULL)),
        NULL
    ));
    return r_label;
}

static TMP_temp codeEmitShiftop(char *operand, TR_exp expression0, TR_exp expression1){
    if(expression0->kind == TR_CONST){
        TMP_temp r_label = TMP_newtemp();
        codeEmit(ASSB_Oper(
            build("%s `d0, `s0, %d", operand, expression0->u.CONST),
            TMPList(r_label, NULL),
            TMPList(munchExpression(expression1), NULL),
            NULL
        ));
        return r_label;
    }
    else if(expression1->kind == TR_CONST){
        TMP_temp r_label = TMP_newtemp();
        codeEmit(ASSB_Oper(
            build("%s `d0, `s0, %d", operand, expression0->u.CONST),
            TMPList(r_label, NULL),
            TMPList(munchExpression(expression0), NULL),
            NULL
        ));
        return r_label;
    }
	codeError("One of the operands for logical shift must be constant");
    return NULL;
}

static void codeEmitCondJump(char *operand, TR_exp treeLeft, TR_exp treeRight, TMP_label trueLabel, TMP_label falseLabel){
	codeEmit(ASSB_Oper(
		build("%s `s0, `s1, j0", operand),
		NULL,
		TMPList(munchExpression(treeLeft),
			TMPList(munchExpression(treeRight), NULL)),
		ASSB_Targets(TMP_LabelList(trueLabel,
			TMP_LabelList(falseLabel, NULL)))
    ));
}

/*let ptrs in callerSaveRegArray point to regs defined in mipsframe.c*/
static inline void initCallerSaveRegs(){
    if(callerSaveRegsNumber == -1){
        callerSaveRegsNumber = 0;
        TMP_tempList t_list;
        t_list = FRM_getRegList(callersaveRegisters);
        while(t_list != NULL && t_list->head != NULL){
            callerSaveRegsNumber++;
            t_list = t_list->tail;
        }
        callerSaveRegArray = check_malloc(callerSaveRegsNumber * sizeof(TMP_temp));
        int count = 0;
        while(t_list != NULL && t_list->head != NULL){
            callerSaveRegArray[count] = t_list->head;
            count++;
            t_list = t_list->tail;
        }
    }
}

static void codeEmitComment(char *msg){
    codeEmit(ASSB_Oper(
        String(msg),
        NULL, NULL, NULL
    ));
}

static void callerSaveRegs(){
    initCallerSaveRegs();
    int count = 0;
    if(callerSaveRegsNumber > 0){
        codeEmitComment("#save registers that callee may use\n");
    }
    for(; count<callerSaveRegsNumber; count++){
        codeEmit(ASSB_Oper(
            build("sw `so, %d(`s1)", thisFRM->offset + -1 * count * FRM_wordSize),
            NULL,
            TMPList(callerSaveRegArray[count], TMPList(FRM_FP(), NULL)),
            NULL
        ));
    }
}

static void callerRestoreRegs(){
    initCallerSaveRegs();
    int count = callerSaveRegsNumber - 1;
    if(callerSaveRegsNumber > 0){
        codeEmitComment("#restore registers saved before\n");
    }
    for(; count >= 0; count--){
       codeEmit(ASSB_Oper( 
            build("ld `d0, %d(`s0)", thisFRM->offset + -1 * count * FRM_wordSize),
            TMPList(callerSaveRegArray[count], NULL),
            TMPList(FRM_FP(), NULL),
            NULL
        ));
    }
}

static TMP_temp munchExpression(TR_exp exp){
    TR_exp expression0 = NULL;
    TR_exp expression1 = NULL;
    TR_exp expression2 = NULL;

    //1. load from memory
    if(exp->kind == TR_MEM){
        expression0 = exp->u.MEM;

        //relative access
        if(expression0->kind == TR_BINOP){
            expression1 = expression0->u.BINOP.left;
            expression2 = expression0->u.BINOP.right;
            if(expression0->u.BINOP.op == TR_plus){
                if(expression1->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("ld `d0, %d(`s0)", expression1->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression2), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else if(expression2->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("ld `d0, %d(`s0)", expression2->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression1), NULL),
                        NULL
                    ));
                    return r_label;
                }
            }
        }
        //absolute access
        else if(expression0->kind == TR_CONST){
            TMP_temp r_label = TMP_newtemp();
            codeEmit(ASSB_Oper(
                build("ld `d0, %d(`s0)", expression0->u.CONST),
                TMPList(r_label, NULL),
                TMPList(FRM_ZERO(), NULL),
                 NULL
            ));
            return r_label;
        }
        //get absolute address from calculation
        else{
            TMP_temp r_label = TMP_newtemp();
            codeEmit(ASSB_Oper(
                build("ld `d0, 0(`s0)"),
                TMPList(r_label, NULL),
                TMPList(munchExpression(expression0), NULL),
                 NULL
            ));
            return r_label;
        }
    }

    //2. BINOP
    else if(exp->kind == TR_BINOP){
        expression0 = exp->u.BINOP.left;
        expression1 = exp->u.BINOP.right;

        switch(exp->u.BINOP.op){
            case TR_plus:
                if(expression0->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("addi `d0, `s0, %d", expression0->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression1), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else if(expression1->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("addi `d0, `s0, %d", expression1->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression0), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else{
                    return codeEmitBinop("add", expression0, expression1);
                }
            case TR_minus:
                return codeEmitBinop("sub", expression0, expression1);
            case TR_mul:
                return codeEmitBinop("mul", expression0, expression1);
            case TR_div:
                return codeEmitBinop("div", expression0, expression1);
            case TR_and:
                if(expression0->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("andi `d0, `s0, %d", expression0->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression1), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else if(expression1->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("andi `d0, `s0, %d", expression1->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression0), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else{
                    return codeEmitBinop("and", expression0, expression1);
                }
            case TR_or:
                if(expression0->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("ori `d0, `s0, %d", expression0->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression1), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else if(expression1->kind == TR_CONST){
                    TMP_temp r_label = TMP_newtemp();
                    codeEmit(ASSB_Oper(
                        build("ori `d0, `s0, %d", expression1->u.CONST),
                        TMPList(r_label, NULL),
                        TMPList(munchExpression(expression0), NULL),
                        NULL
                    ));
                    return r_label;
                }
                else{
                    return codeEmitBinop("or", expression0, expression1);
                }
            case TR_lshift:
                return codeEmitShiftop("sll", expression0, expression1);
            case TR_rshift:
                return codeEmitShiftop("srl", expression0, expression1);
            case TR_arshift:
                return codeEmitShiftop("sra", expression0, expression1);
            case TR_xor:
                return codeEmitShiftop("xor", expression0, expression1);
            default:
                codeError("unexpected (illegal) binary operator");
        }
    }

    //3. TEMP
    else if(exp->kind == TR_TEMP){
        return exp->u.TEMP;
    }

    //4. NAME
    //TR_exp->u.NAME is instance of TMP_label
    else if(exp->kind == TR_NAME){
        TMP_temp r_label = TMP_newtemp();
        codeEmit(ASSB_Oper(
            build("la `d0, %s", TMP_labelstring(exp->u.NAME)),
            TMPList(r_label, NULL),
            NULL,
            NULL
        ));
        return r_label;
    }

    //5. CONST
    else if(exp->kind == TR_CONST){
        TMP_temp r_label = TMP_newtemp();
        codeEmit(ASSB_Oper(
            build("li `d0, %s", exp->u.CONST),
            TMPList(r_label, NULL),
            NULL,
            NULL
        ));
        return r_label;
    }

    //6. CALL
    else if(exp->kind == TR_CALL){
        int argNum = 0;
        TMP_tempList t_list = munchArguments(exp->u.CALL.args, &argNum);
        expression0 = exp->u.CALL.fun;
        //The jump target must be a label
        if(expression0->kind == TR_NAME){

            //1) save regs
            callerSaveRegs();
            
            //2) pass arguments
            if(argNum > 0){
                codeEmitComment("#place arguments into registers and stack (from the 5th one, if any)\n");	
            }
            int count;
            TMP_tempList argList;
            //put first four arguments into registers
            for(count = 0, argList = t_list; count < 4 && count < argNum; count++, argList = argList->tail){
                codeEmit(ASSB_Oper(
                    build("move `d0, `s0"),
                    TMPList(FRM_AN(count), NULL),
                    TMPList(argList->head, NULL),
                    NULL
                ));
            }
            //put the rest arguments into stack in reversed order
            count = 0;
            argNum = argNum - 4;
            for(;argList != NULL; argList = argList->tail, count++){
                codeEmit(ASSB_Oper(
                    build("sw `s0, %d(`s1)", thisFRM->offset + -1 * (callerSaveRegsNumber - 1) * FRM_wordSize + -1 * (argNum - count) * FRM_wordSize),
                    NULL,
                    TMPList(argList->head, TMPList(FRM_FP(), NULL)),
                    NULL
                ));
            }

            //call
            codeEmitComment("#registers saved; call function now\n");
            codeEmit(ASSB_Oper(
                build("jal %s", TMP_labelstring(expression0->u.NAME)),
                getCalldefs(),
                t_list,
                NULL
            ));

            //restore regs
            callerRestoreRegs();

            return FRM_RV();

        }
        codeError("unexpected or illegal target for procedure call");
    }
    else{
        codeError("unexpected or illegal expression type");
    }
    return NULL;
}

static void munchStatement(TR_stm stm){
    TR_exp expression0 = NULL;
    TR_exp expression1 = NULL;
    TR_exp expression2 = NULL;
    TR_exp expression3 = NULL;

    //1. store
    if(stm->kind == TR_MOVE){
        expression0 = stm->u.MOVE.dst;
        expression3 = stm->u.MOVE.src;

        //to memory
        //MOVE(MEM(... ...), e3)
        if(expression0->kind == TR_MEM){
            expression0 = expression0->u.MEM;

            //move to relative access
            if(expression0->kind == TR_BINOP){
                expression1 = expression0->u.BINOP.left;
                expression2 = expression0->u.BINOP.left;
                if(expression0->u.BINOP.op == TR_plus){
                    if(expression1->kind == TR_CONST){
                        //MOVE(MEM(BINOP(PLUS, e2, CONST(i))), e3)
                        codeEmit(ASSB_Oper(
                            build("sw `s0, %d(`s1)", expression1->u.CONST),
                            NULL,
                            TMPList(munchExpression(expression3), TMPList(munchExpression(expression2), NULL)),
                            NULL
                        ));
                        return;
                    }
                    else if(expression2->kind == TR_CONST){
                        codeEmit(ASSB_Oper(
                            build("sw `s0, %d(`s1)", expression2->u.CONST),
                            NULL,
                            TMPList(munchExpression(expression3), TMPList(munchExpression(expression1), NULL)),
                            NULL
                        ));
                        return;
                    }
                }
            }

            //absolute access
            else if(expression0->kind == TR_CONST){
                //MOVE(MEM(CONST(i)), e3)
                //     |--- e0 ---|
                codeEmit(ASSB_Oper(
                    build("sw `s0, %d(`s1)", expression0->u.CONST),
                    NULL,
                    TMPList(munchExpression(expression3), TMPList(FRM_ZERO(), NULL)),
                    NULL
                ));
                return;
            }

            //MOVE(MEM(... ...), e3)
            //        |-- e0 -|
            codeEmit(ASSB_Oper(
                build("sw `s0, 0(`s1)"),
                NULL,
                TMPList(munchExpression(expression3), TMPList(munchExpression(expression0), NULL)),
                NULL
            ));
            return;
        }
        //move to registers
        //MOVE(TEMP(...), e3)
        //     /--e0--/
        else if(expression0->kind == TR_TEMP){
            if(expression3->kind == TR_TEMP){
                codeEmit(ASSB_Move(
                    build("move `d0, `s0"),
                    TMPList(expression0->u.TEMP, NULL),
                    TMPList(expression3->u.TEMP, NULL)
                ));
                return;
            }
            //general case
            codeEmit(ASSB_Move(
                build("move `d0, `s0"),
                TMPList(expression0->u.TEMP, NULL),
                TMPList(munchExpression(expression3), NULL)
            ));
            return;
        }
        codeError("illegal destination for MOVE");
    }

    //2. jump without condition
    else if(stm->kind == TR_JUMP){
        expression0 = stm->u.JUMP.exp;
        TMP_labelList jumpLabels = stm->u.JUMP.jumps;
        if(expression0->kind == TR_NAME){
            codeEmit(ASSB_Oper(
				build("j `j0"), 
				NULL,						
				NULL,
				ASSB_Targets(jumpLabels)	//j0	Temp_LabelList(exp0->u.NAME, NULL)
			));
			return;
        }
        codeError("illegal target for JUMP: can only be a LABEL");
        return;
    }

    //3. jump with condition
    else if(stm->kind == TR_CJUMP){
        expression0 = stm->u.CJUMP.left;
        expression1 = stm->u.CJUMP.right;
        TMP_label trueLabel = stm->u.CJUMP.true;
        TMP_label falseLabel = stm->u.CJUMP.false;
        switch(stm->u.CJUMP.op){
            case TR_eq:
                codeEmitCondJump("beq", expression0, expression1, trueLabel, falseLabel);
				return;
            case TR_ne:
                codeEmitCondJump("neq", expression0, expression1, trueLabel, falseLabel);
				return;
            case TR_lt:
                codeEmitCondJump("blt", expression0, expression1, trueLabel, falseLabel);
				return;
            case TR_gt:
                codeEmitCondJump("bgt", expression0, expression1, trueLabel, falseLabel);
				return;
            case TR_le:
                codeEmitCondJump("ble", expression0, expression1, trueLabel, falseLabel);
                return;
            case TR_ge:
                codeEmitCondJump("bge", expression0, expression1, trueLabel, falseLabel);
                return;
            default:
                codeError("unexpected relational operator; only eq, ne, lt, gt, le, ge are supported so far");
        }
        return;
    }

    //4. label
    else if(stm->kind == TR_LABEL){
        codeEmit(ASSB_Label(build("%s: ", TMP_labelstring(stm->u.LABEL)), stm->u.LABEL));
        return;
    }

    //5. expression
    else if(stm->kind == TR_EXP){
        munchExpression(stm->u.EXP);
        return;
    }

    //Errors
	codeError("unexpected or illegal statement type");
}

static void codeEmit(ASSB_instr instruction){
    if(instructionLast == NULL){
        instructionList = ASSB_InstrList(instruction, NULL);
        instructionLast = instructionList;
    }
    else{
        instructionLast->tail = ASSB_InstrList(instruction, NULL);
        instructionLast = instructionLast->tail;
    }
}

static TMP_tempList TMPList(TMP_temp head, TMP_tempList tail){
    return TMP_TempList(head, tail);
}

ASSB_instrList FRM_codegen(FRM_frame frm, TR_stmList statementList){
    ASSB_instrList t_list;
    TR_stmList stmL;
    thisFRM = frm;

    for(stmL = statementList; stmL != NULL; stmL = stmL->tail){
        munchStatement(stmL->head);
    }
    t_list = instructionList;

    instructionLast = NULL;
    instructionList = NULL;

    return FRM_procEntryExit2(t_list);
}

//store some regs that are still active when function return
static TMP_tempList returnSink = NULL;

static ASSB_instrList FRM_procEntryExit2(ASSB_instrList by){
    if(returnSink == NULL){
        returnSink = TMP_TempList(FRM_ZERO(),
                        TMP_TempList(FRM_RA(),
                        TMP_TempList(FRM_RV(),
                        TMP_TempList(FRM_SP(),NULL
                        ))));
    }
    return ASSB_splice(by, ASSB_InstrList(ASSB_Oper("", NULL, returnSink, NULL), NULL));
}

ASSB_proc FRM_procEntryExit3(FRM_frame frm, ASSB_instrList by){
    char buffer[80];
    sprintf(buffer, "PROCEDURE %s\n", SB_name(FRM_name(frm)));
    return ASSB_Proc(String(buffer), by, "END\n");
}

