#ifndef _ERRORMSG_H_
#define _ERRORMSG_H_
extern bool ERRMSG_anyErrors;

void ERRMSG_newline(void);

extern int ERRMSG_tokPos;

void ERRMSG_error(int, string,...);
void ERRMSG_impossible(string,...);
void ERRMSG_reset(string filename);
#endif
