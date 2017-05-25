/*
* types.h
*
* All types and functions declared in this header file begin with "TP_"
*/

#ifndef _TYPES_H_
#define _TYPES_H_

typedef struct TP_tp_ *TP_tp;

struct TP_tp_ {
    enum {
	TP_int,
	TP_string,
	TP_array,
	TP_void
    } kind;
    union {
	TP_tp array;
    }u;
};

TP_tp TP_Int(void);
TP_tp TP_String(void);
TP_tp TP_Void();
TP_tp TP_Array(TP_tp tp);

#endif // !_TYPES_H_


