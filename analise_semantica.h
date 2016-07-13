#ifndef __ANALISE_SEMANTICA_H__
#define __ANALISE_SEMANTICA_H__

#include "absyn.h"
#include "types.h"
#include "table.h"
#include "symbol.h"


typedef void *Tr_exp;
struct expty {
  Tr_exp exp;
  Ty_ty ty;
};

/* there are 2 types of bindings in tiger:
 * type binding: map symbol to Ty_ty
 * variable binding: map symbol to E_enventry
 */
typedef struct E_enventry_ *E_enventry;
struct E_enventry_ {
  enum {
    E_varEntry,
    E_funEntry
  } kind;
  union {
    struct { Ty_ty ty; } var;
    struct { Ty_tyList formals; Ty_ty result; } fun;
  } u;
};


struct expty ExpTy(Tr_exp exp, Ty_ty ty);

struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty analisaExp(S_table vf_table, S_table ty_table, A_exp e);

struct expty transCallExp(S_table venv, S_table tenv, A_exp e);
struct expty transOpExp(S_table venv, S_table tenv, A_exp e);
struct expty transRecordExp(S_table venv, S_table tenv, A_exp e);
struct expty transSeqExp(S_table venv, S_table tenv, A_expList e);
struct expty transAssignExp(S_table venv, S_table tenv, A_exp e);
struct expty transIfExp(S_table venv, S_table tenv, A_exp e);
struct expty transWhileExp(S_table venv, S_table tenv, A_exp e);
struct expty transForExp(S_table venv, S_table tenv, A_exp e);
struct expty transBreakExp(S_table venv, S_table tenv, A_exp e);
struct expty transLetExp(S_table venv, S_table tenv, A_exp e);
struct expty transArrayExp(S_table venv, S_table tenv, A_exp e);
struct expty transExpExp(S_table vf_table, S_table ty_table,A_exp exp);

void transDec(S_table venv, S_table tenv, A_dec d, A_decList tail);
void transFuncDec(S_table venv, S_table tenv, A_dec d, A_decList tail);
void transVarDec(S_table venv, S_table tenv, A_dec d);
void transTypeDec(S_table venv, S_table tenv, A_dec d, A_decList tail);
Ty_ty transTy(S_table ty_table, S_symbol ty_name, A_ty t, A_decList tail);
Ty_tyList makeFormalTyList(S_table tenv, A_fieldList fieldList);

void SEM_transProg(A_exp exp, int *isErrors);

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);


S_table initializeTypesTable(void);

S_table initializeVariablesAndFunctionsTable(void);


Ty_tyList parseParamListToTyList(S_table ty_table, A_paramList parametros);
Ty_ty getTypeForUntypedVar(S_table vf_table, S_table ty_table, A_exp exp);
Ty_ty getPrimitiveType(Ty_ty t);

#endif
