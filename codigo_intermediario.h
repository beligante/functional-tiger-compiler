#ifndef __CODIGO_INTERMEDIARIO_H__
#define __CODIGO_INTERMEDIARIO_H__

#include "absyn.h"
#include "types.h"
#include "table.h"
#include "symbol.h"

typedef struct  string_list_ *string_list;

struct string_list_ {
  char *head;
  string_list tail;
};

void geraCodigoIntermediario(string filename, A_exp e);
char *evaluateExp(FILE* arquive, A_exp e, int level);
char *evaluateSeqExp(FILE* arquive, A_expList el, int level);
char *evaluateLetExp(FILE* arquive, A_exp e, int level);
char *evaluateExpExp(FILE* arquive, A_exp e, int level);
char *evaluateVar(FILE* arquive, A_var v, int level);
char *evaluateCallExp(FILE* arquive, A_exp e, int level);
char * evaluateOpExp(FILE* arquive, A_exp e, int level);
char *evaluateRecordExp(FILE* arquive, A_exp e, int level);
char *evaluateAssignExp(FILE* arquive, A_exp e, int level);
char *evaluateIfExp(FILE* arquive, A_exp e, int level);
char *evaluateArrayExp(FILE* arquive, A_exp e, int level);
char *evaluateDec(FILE* arquive, A_dec d, int level);
char *evaluateFuncDec(FILE* arquive, A_dec d, int level);
void getParams(A_fieldList fieldList, char **count);
string_list stringList(char *head, string_list tail);
S_table initializeUnicodeTable();
char * evaluateStringExp(FILE* arquive, string e, int level);
void printGlobalStringList(FILE *arquive);
char* subString(const char* input, int offset, int len, char* dest);
char *evaluateVarDec(FILE* arquive, A_dec d, int level);
int getRealKind(A_exp e);
void setVarsFunDec(A_fieldList fl, Ty_tyList tl);
int isPrintLoad(A_exp e);
#endif
