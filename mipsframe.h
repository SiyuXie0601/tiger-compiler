/*********************************************************************************
*FileName: mipsframe.h
*Author: Debbie Xie
*Create Time: 2017/5/29
*Description: TIGER'S IR Translation Module.
**********************************************************************************/

#ifndef _MIPSFRAME_H_
#define _MIPSFRAME_H_

struct FRM_frame_ {
	FRM_accessList formals;
	int locals;
	int offset;
	TMP_label begin_label;
};

struct FRM_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset;
		TMP_temp reg;
	} u;
};

#endif