/*********************************************************************************
*FileName: mipsframe.c
*Author: Debbie Xie
*Create Time: 2017/5/29
*Description: TIGER'S IR Translation Module.
			  fragList stores strings'(label, str) 
			  and functions'(body, frame) label
**********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "util.h"
#include "mipsframe.h"

const int FRM_wordSize = 4;

static TMP_temp fp = NULL;

/*return the frame pointer
 *if there is none, new*/
 TMP_temp FRM_FP(){
    if(fp == NULL){
        fp = TMP_newtemp();
    }
    return fp;
 }

 static TMP_temp ra = NULL;

/*return the return address register*/
 TMP_temp FRM_RA(){
     if(ra == NULL){
         ra = TMP_newtemp();
     }
     return ra;
 }

 static TMP_temp rv = NULL;
/*return the return value register*/
 TMP_temp FRM_RV(){
     if(rv == NULL){
         rv = FRM_VN(0);
     }
     return rv;
 }

static TMP_temp sp = NULL;
/*return the stack register*/
TMP_temp FRM_SP(){
	if(sp == NULL){
		sp = TMP_newtemp();
	}
	return sp;
}

static TMP_temp zero = NULL;
/*return the zero constant register*/
TMP_temp FRM_ZERO(){
	if(zero==NULL){
		zero = TMP_newtemp();
	}
	return zero;
}

static TMP_temp sn[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static TMP_temp tn[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static TMP_temp an[4] = {NULL, NULL, NULL, NULL};
static TMP_temp vn[2] = {NULL, NULL};

TMP_temp FRM_SN(int index){
	if(0<=index && index<=7) {
		if(sn[index]==NULL){
			sn[index] = TMP_newtemp();
		}
		return sn[index];
	} else {
		printf("Internal error: using a register index pointer that is not available for MIPS.");
		exit(2);
	}
}

TMP_temp FRM_TN(int index){
	if(0<=index && index<=9) {
		if(tn[index]==NULL){
			tn[index] = TMP_newtemp();
		}
		return tn[index];
	} else {
		printf("Internal error: using a register index pointer that is not available for MIPS.");
		exit(2);
	}
}

TMP_temp FRM_AN(int index){
	if(0<=index && index<=3) {
		if(an[index]==NULL){
			an[index] = TMP_newtemp();
		}
		return an[index];
	} else {
		printf("Internal error: using a register index pointer that is not available for MIPS.");
		exit(2);
	}
}

TMP_temp FRM_VN(int index){
	if(0<=index && index<=1) {
		if(vn[index]==NULL){
			vn[index] = TMP_newtemp();
		}
		return vn[index];
	} else {
		printf("Internal error: using a register index pointer that is not available for MIPS.");
		exit(2);
	}
}

static TMP_map _FRM_tempMap = NULL;

/*Will be called in main.c
 *Add register's temp label in TAB_table*/
TMP_map FRM_tempMap(){
    if(_FRM_tempMap == NULL){
        _FRM_tempMap = TMP_empty();
        TMP_enter(_FRM_tempMap, FRM_RV(),String("$v0"));
        TMP_enter(_FRM_tempMap, FRM_RA(),String("$ra"));
        TMP_enter(_FRM_tempMap, FRM_FP(),String("$fp"));
        TMP_enter(_FRM_tempMap, FRM_SP(),String("$sp"));
        TMP_enter(_FRM_tempMap, FRM_ZERO(),String("$zero"));
        int index;
        for(index = 0; index < 8; index++){
            char* name = check_malloc(8*sizeof(char));
            sprintf(name, "$s%d", index);
            TMP_enter(_FRM_tempMap, FRM_SN(index), name);
        }
        for(index = 0; index < 10; index++){
            char* name = check_malloc(8*sizeof(char));
            sprintf(name, "$t%d", index);
            TMP_enter(_FRM_tempMap, FRM_TN(index), name);
        }
        for(index = 0; index < 4; index++){
            char* name = check_malloc(8*sizeof(char));
            sprintf(name, "$a%d", index);
            TMP_enter(_FRM_tempMap, FRM_AN(index), name);
        }
    }
    return _FRM_tempMap;
}

static TMP_tempList registerLists[4] = {NULL, NULL, NULL, NULL};

/*will be called in mipcodegen.c
 *Get one of four group of regs' TMP_temp*/
TMP_tempList FRM_getRegList(RList_type type){
	int index = (int)type;
	if(registerLists[index] == NULL){
		switch(type){
			case specialRegisters:
				registerLists[index] = TMP_TempList(
										FRM_RV(), TMP_TempList(
											FRM_RA(), TMP_TempList(
												FRM_FP(), TMP_TempList(
													FRM_SP(), NULL
												)
											)
										)
				                   );
				break;
			case argRegisters:
				registerLists[index] = TMP_TempList(
										FRM_AN(0), TMP_TempList(
											FRM_AN(1), TMP_TempList(
												FRM_AN(2), TMP_TempList(
													FRM_AN(3), NULL
												)
											)
										)
				                   );
				break;
			case calleesaveRegisters:
				registerLists[index] = TMP_TempList(
										FRM_SN(0), TMP_TempList(
											FRM_SN(1), TMP_TempList(
												FRM_SN(2), TMP_TempList(
													FRM_SN(3), TMP_TempList(
														FRM_SN(4), TMP_TempList(
															FRM_SN(5), TMP_TempList(
																FRM_SN(6), TMP_TempList(
																	FRM_SN(7), NULL
																)
															)
														)
													)
												)
											)
										)
				                   );
				break;
			case callersaveRegisters:
				registerLists[index] = TMP_TempList(
										FRM_TN(0), TMP_TempList(
											FRM_TN(1), TMP_TempList(
												FRM_TN(2), TMP_TempList(
													FRM_TN(3), TMP_TempList(
														FRM_TN(4), TMP_TempList(
															FRM_TN(5), TMP_TempList(
																FRM_TN(6), TMP_TempList(
																	FRM_TN(7), NULL
																)
															)
														)
													)
												)
											)
										)
				                   );
				break;
		}
	}
	return registerLists[index];
}

void FRM_printFrag(FRM_frag frag);

FRM_frag FRM_StringFrag(TMP_label strlabel, string str){
	FRM_frag strFrag = (FRM_frag)check_malloc(sizeof(struct FRM_frag_));
	strFrag->kind = FRM_stringFrag;
	strFrag->u.stringg.label = strlabel;
	strFrag->u.stringg.str = str;
	return strFrag;
}

FRM_frag FRM_ProcFrag(TR_stm funBody, FRM_frame frame){
	FRM_frag procFrag = (FRM_frag)check_malloc(sizeof(struct FRM_frag_));
	procFrag->kind = FRM_procFrag;
	procFrag->u.proc.body = funBody;
	procFrag->u.proc.frame = frame;
	return procFrag;
}

FRM_fragList FRM_FragList(FRM_frag key, FRM_fragList tail){
	FRM_fragList fList = (FRM_fragList)check_malloc(sizeof(struct FRM_fragList_));
	fList->head = key;
	fList->tail = tail;
	return fList;
}

static FRM_fragList *fragList = NULL;
static FRM_fragList fragList_head = NULL;

static FRM_frag* extendFragList(){
	if(fragList == NULL){
		fragList = (FRM_fragList*)check_malloc(sizeof(FRM_fragList*));
	}
	*fragList = (FRM_fragList)check_malloc(sizeof(struct FRM_fragList_));

	if(fragList_head == NULL){
		fragList_head = *fragList;
	}
	FRM_frag *curFrag = &((*fragList)->head);
	fragList = &((*fragList)->tail);
	*fragList = NULL;
	
	return curFrag;
}

void FRM_String(TMP_label strLabel, string str){
	FRM_frag *curFrag = extendFragList();
	*curFrag = FRM_StringFrag(strLabel, str);
}

void FRM_Proc(TR_stm funBody, FRM_frame frame){
	FRM_frag *curFrag = extendFragList();
	*curFrag = FRM_ProcFrag(funBody, frame);
}

FRM_fragList FRM_getFragList(){
	return fragList_head;
}


/*produce IR tree(TR_Mem) to access target address*/
TR_exp FRM_Exp(FRM_access acc, TR_exp frmPtr){
	/*variable is in frame*/
	if(acc->kind == inFrame){
		return TR_Mem(TR_Binop(TR_plus, frmPtr, TR_Const(acc->u.offset)));
	}
	/*variable is in register*/
	else{
		return TR_Temp(acc->u.reg);
	}
}

static FRM_access InFrame(int frameOffset){
	FRM_access access = (FRM_access)check_malloc(sizeof(struct FRM_access_));
	access->kind = inFrame;
	access->u.offset = frameOffset;
	return access;
}

static FRM_access InReg(TMP_temp reg){
	FRM_access access = (FRM_access)check_malloc(sizeof(struct FRM_access_));
	access->kind = inReg;
	access->u.reg = reg;
	return access;
}

static int getNextLoc(FRM_frame frm){
	return frm->offset;
}

static TMP_temp getNextReg(FRM_frame frm){
	frm->locals++;
	return TMP_newtemp();
}

static FRM_access static_access = NULL;
FRM_access FRM_staticLink(){
	if(static_access == NULL){
		static_access = InFrame(0);
	}
	return static_access;
}

FRM_frame FRM_newFrame(TMP_label frmName, UN_boolList argFormals){
	/*new a frame*/
	FRM_frame frm = (FRM_frame)check_malloc(sizeof(struct FRM_frame_));
	frm->offset = 0;
	frm->locals = 0;
	frm->begin_label = frmName;

	FRM_accessList accessList = NULL;
	FRM_accessList accessList_head = NULL;
	/*add all args into frame's accessList*/
	for(; argFormals != NULL; argFormals = argFormals->tail){
		if(accessList_head == NULL){
			accessList = (FRM_accessList)check_malloc(sizeof(struct FRM_accessList_));
			accessList_head = accessList;
		} 
		else {
			accessList->tail = (FRM_accessList)check_malloc(sizeof(struct FRM_accessList_));
			accessList = accessList->tail;
		}

		if(argFormals->head == TRUE){
		/*store it in memory*/
			accessList->head = InFrame(frm->offset);
			frm->offset -= FRM_wordSize;
		} 
		else {
		/*store it in register*/
			accessList->head = InReg(getNextReg(frm));
		}
	}

	if (accessList != NULL) {
		accessList->tail = NULL;
	}
	frm->formals = accessList_head;
	return frm;

} 

TMP_label FRM_name(FRM_frame frm){
	return frm->begin_label;
}

FRM_accessList FRM_formals(FRM_frame frm){
	return frm->formals;
}

FRM_access FRM_allocLocal(FRM_frame frm, bool escape){
	FRM_access access;
	if(escape == TRUE){
	/*store in memory*/
		access = InFrame(frm->offset);
		frm->offset -= FRM_wordSize;
	} 
	/*store in register*/
	else {
		access = InReg(getNextReg(frm));
	}
	return access;
}

TR_exp FRM_externalCall(string name, TR_expList arguments){
	return TR_Call(TR_Name(TMP_namedlabel(name)), arguments);
}

/*return an escapelist with all TRUES*/
static void createEscapeList(UN_boolList *escs, int index){
	int node_size = (sizeof(struct UN_boolList_))/8;
	UN_boolList ptr = (UN_boolList)check_malloc(index *sizeof(struct UN_boolList_));
	UN_boolList ptr_head = ptr;

	for(;index > 0; index--){
		ptr->head = TRUE;
		if(index > 1){
			ptr->tail = ptr + node_size;
			ptr = ptr->tail;
		}
		else{
			ptr->tail = NULL;
		}
	}
	*escs = ptr_head;
}




















