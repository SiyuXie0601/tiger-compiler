/*********************************************************************************
*FileName: codegen.h
*Author: Debbie Xie
*Create Time: 2017/6/16
*Description: Header for the codegen phase
**********************************************************************************/

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

ASSB_instrList FRM_codegen(FRM_frame frm, TR_stmList statementList);

ASSB_proc FRM_procEntryExit3(FRM_frame frm, ASSB_instrList by);

#endif