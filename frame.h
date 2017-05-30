/*********************************************************************************
*FileName: frame.h
*Author: Debbie Xie
*Create Time: 2017/5/29
*Description: TIGER'S IR Translation Module.
**********************************************************************************/

#ifndef _FRAME_H_
#define _FRAME_H_

/*return Registers' TMP_temp*/
 TMP_temp FRM_FP();
 TMP_temp FRM_RA();
 TMP_temp FRM_RV();
 TMP_temp FRM_SP();
 TMP_temp FRM_ZERO();
 TMP_temp FRM_SN(int index);
 TMP_temp FRM_TN(int index); 
 TMP_temp FRM_AN(int index);
 TMP_temp FRM_VN(int index);

/*Will be called in main.c
 *Add register's temp label in TAB_table*/
TMP_map _FRM_tempMap();

typedef enum {
	specialregs, 	//special regs like FP, RV, RA
	argregs, 	//regs to pass arguments
	calleesaves, 	//regs that callee must not change (or save-restore)
	callersaves	//regs that caller must preserve since callee may affect them
} RList_type ;

/*will be called in mipcodegen.c
 *Get one of four group of regs' TMP_temp*/
TMP_tempList FRM_getRegList(RList_type type);




/*in mipsframe.c
 *There is a static framelist who stores FRM_frag's list*/
typedef struct FRM_frag_ *FRM_frag;
struct FRM_frag_ {
	enum {FRM_stringFrag, FRM_procFrag} kind;
	union {
		struct {
			TMP_label label;
			string str;
		} stringg;
		struct {
			TR_stm body;
			FRM_frame frame;
		} proc;
	} u;
};
typedef struct FRM_fragList_ *FRM_fragList;
struct FRM_fragList_ {
	FRM_frag head;
	FRM_fragList tail;
};

void FRM_printFrag(FRM_frag frag);
FRM_frag FRM_StringFrag(TMP_label strlabel, string str);
FRM_frag FRM_ProcFrag(TR_stm funBody, FRM_frame frame);
FRM_fragList FRM_FragList(FRM_frag key, FRM_fragList tail);
static FRM_frag* extendFragList();
void FRM_String(TMP_label strLabel, string str);
void FRM_Proc(TR_stm funBody, FRM_frame frame);
FRM_fragList FRM_getRegList();





/*FRM_access_ and FRM_frame_ are defined in mipsframe.h*/
typedef struct FRM_frame_ *FRM_frame;
typedef struct FRM_access_ *FRM_access;

typedef struct FRM_accessList_ *FRM_accessList;
struct FRM_accessList_ {
	FRM_access head;
	FRM_accessList tail;
};
extern const int FRM_wordSize;
TR_exp FRM_Exp(FRM_access acc, TR_exp frmPtr);
FRM_access FRM_staticLink();
FRM_frame FRM_newFrame(TMP_label frmName, U_boolList argFormals);
TMP_label FRM_name(FRM_frame frm);
FRM_accessList FRM_formals(FRM_frame frm);
FRM_access FRM_allocLocal(FRM_frame frm, bool escape);
TR_exp FRM_externalCall(string name, TR_expList arguments);
#endif