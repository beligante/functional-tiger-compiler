#ifndef __ABSYN_H__
#define __ABSYN_H__

#include "symbol.h"
#include "util.h"

/*
 * absyn.h - Abstract Syntax Header (Chapter 4)
 *
 * All types and functions declared in this header file begin with "A_"
 * Linked list types end with "..list"
 */

/* Type Definitions */

typedef int A_pos;

typedef struct A_var_ *A_var;
typedef struct A_exp_ *A_exp;
typedef struct A_dec_ *A_dec;
typedef struct A_ty_ *A_ty;
typedef struct A_decList_ *A_decList;
typedef struct A_expList_ *A_expList;
typedef struct A_field_ *A_field;
typedef struct A_fieldList_ *A_fieldList;
typedef struct A_efield_ *A_efield;
typedef struct A_efieldList_ *A_efieldList;
typedef struct A_paramList_ *A_paramList;

typedef enum {
  A_plusOp,
  A_minusOp,
  A_timesOp,
  A_divideOp,
  A_eqOp,
  A_neqOp,
  A_ltOp,
  A_leOp,
  A_gtOp,
  A_geOp
} A_oper;

// lvalue
struct A_var_
{
  enum {
    A_simpleVar,
    A_fieldVar,
    A_subscriptVar
  } kind;
  A_pos pos;
  union {
    S_symbol simple;
    struct {
      A_var var;
      S_symbol sym;
    } field;
    struct {
      A_var var;
      A_exp exp;
    } subscript;
  } u;
};

struct A_exp_
{
  enum {
    A_varExp,
    A_nilExp,
    A_intExp,
    A_stringExp,
    A_callExp,
    A_opExp,
    A_recordExp,
    A_seqExp,
    A_assignExp,
    A_ifExp,
    A_whileExp,
    A_forExp,
    A_breakExp,
    A_letExp,
    A_expExp,
    A_arrayExp
  } kind;
  A_pos pos;
  union {
    A_var var;
    A_exp expp;
    /* nil; - needs only the pos */
    int intt;
    string stringg;
    struct { S_symbol func; A_expList args; } call;
    struct { A_oper oper; A_exp left; A_exp right; } op;
    struct { S_symbol typ; A_efieldList fields; } record;
    A_expList seq;
    struct { A_var var; A_exp exp; } assign;
    struct { A_exp test, then, elsee; } iff; /* elsee is optional */
    struct { A_exp test, body; } whilee;
    struct { S_symbol var; A_exp lo, hi, body; bool escape; } forr;
    /* breakk; - need only the pos */
    struct { A_decList decs; A_expList body; } let;
    struct { S_symbol typ; A_exp size, init; } array;
    struct { A_exp exp; A_expList args; } anonfunc;
  } u;
  A_exp right;
};

// declaration node
struct A_dec_ 
{
  enum {
    A_functionDec,
    A_varDec,
    A_typeDec
  } kind;
  A_pos pos;
  union {
    struct {
      S_symbol name;
      A_fieldList params; 
      S_symbol result;
      A_exp body;
    } function;
    /* escape may change after the initial declaration */
    struct {
      S_symbol var;
      S_symbol typ;
      A_exp init;
      bool escape;
    } var;
    struct {
      S_symbol name;
      A_ty ty;
    } type;
  } u;
};
struct A_ty_ {
  enum {
    A_nameTy,
    A_recordTy,
    A_arrayTy,
    A_funcTyFuncReturn,
    A_funcTySymReturn
  } kind;
  union {
    S_symbol name;
    A_fieldList record;
    S_symbol array;
  } u;

  union{
    A_ty  ty;
    /*Sera uma lista com apenas um elemento*/A_paramList sym;
  } returns;
  A_paramList parameter;
};

/* Linked lists and nodes of lists */

struct A_paramList_ {
  S_symbol head;
  A_pos pos;
  A_paramList tail;
};

struct A_field_ {
  S_symbol name, typ;
  A_pos pos;
  bool escape;
};
struct A_fieldList_ {
  A_field head;
  A_fieldList tail;
};
struct A_expList_ {
  A_exp head;
  A_expList tail;
};

struct A_decList_ {
  A_dec head;
  A_decList tail;
};
struct A_efield_ {
  S_symbol name;
  A_exp exp;
};
struct A_efieldList_ {
  A_efield head;
  A_efieldList tail;
};

/* Function Prototypes */
A_var A_SimpleVar(A_pos pos, S_symbol sym);
A_var A_FieldVar(A_pos pos, A_var var, S_symbol sym);
A_var A_SubscriptVar(A_pos pos, A_var var, A_exp exp);

A_exp A_VarExp(A_pos pos, A_var var);
A_exp A_ExpExp(A_exp actual, A_exp right);
A_exp A_NilExp(A_pos pos);
A_exp A_IntExp(A_pos pos, int i);
A_exp A_StringExp(A_pos pos, string s);
A_exp A_CallExp(A_pos pos, S_symbol func, A_expList args);
A_exp A_CallExpAnonFunc(A_pos pos, A_exp func, A_expList args);
A_exp A_OpExp2(A_pos pos, A_oper oper, A_exp left, A_exp right, A_exp rightExp);
A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right);
A_exp A_RecordExp(A_pos pos, S_symbol typ, A_efieldList fields);
A_exp A_SeqExp(A_pos pos, A_expList seq);
A_exp A_AssignExp(A_pos pos, A_var var, A_exp exp);
A_exp A_IfExp2(A_pos pos, A_exp test, A_exp then, A_exp elsee, A_exp rightExp);
A_exp A_IfExp(A_pos pos, A_exp test, A_exp then, A_exp elsee);
A_exp A_WhileExp(A_pos pos, A_exp test, A_exp body);
A_exp A_ForExp(A_pos pos, S_symbol var, A_exp lo, A_exp hi, A_exp body);
A_exp A_BreakExp(A_pos pos);
A_exp A_LetExp(A_pos pos, A_decList decs, A_expList body);
A_exp A_ArrayExp(A_pos pos, S_symbol typ, A_exp size, A_exp init);

A_dec A_VarDec(A_pos pos, S_symbol var, S_symbol typ, A_exp init);
A_dec A_Fundec(A_pos pos, S_symbol name, A_fieldList params, S_symbol result, A_exp body);
A_dec A_TypeDec(A_pos pos, S_symbol name, A_ty ty);

A_ty A_NameTy(S_symbol name);
A_ty A_RecordTy(A_fieldList record);
A_ty A_ArrayTy(S_symbol array);
A_ty A_FuncTySimpleTypeReturn(A_paramList parameter, A_paramList returns);
A_ty A_FuncTyFuncReturn(A_paramList parameter, A_ty returns);

A_paramList A_ParameterList(A_pos pos, S_symbol head, A_paramList tail);

A_field A_Field(A_pos pos, S_symbol name, S_symbol typ);
A_fieldList A_FieldList(A_field head, A_fieldList tail);

A_expList A_ExpList(A_exp head, A_expList tail);
A_decList A_DecList(A_dec head, A_decList tail);
A_efield A_Efield(S_symbol name, A_exp exp);
A_efieldList A_EfieldList(A_efield head, A_efieldList tail);

#endif
