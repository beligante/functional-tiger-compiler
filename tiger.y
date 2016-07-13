%{
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"

#define YYDEBUG 1

int yylex(void); /* function prototype */

A_exp absyn_root;

void yyerror(char *s)
{
  EM_error(EM_tokPos, "%s", s);
}

%}

%union {
  int pos;
  int ival;
  string sval;
  A_var var;
  A_exp exp;
  A_dec dec;
  A_ty ty;
  A_decList decList;
  A_expList expList;
  A_field field;
  A_fieldList fieldList;
  A_efield efield;
  A_efieldList efieldList;
  A_paramList paramList;
}

%token <sval> ID STRING
%token <ival> INT

%type <exp> exp program funcall rec_creation exp_or exp_or_rec exp_and exp_and_rec exp_compa exp_add exp_add_rec exp_mul exp_mul_rec exp_nega exp_seq factor
%type <var> lvalue
%type <expList> argseq expseq
%type <efieldList> efield_seq
%type <efield> efield
%type <decList> decs
%type <dec> dec tydec vardec fundec
%type <ty> ty func_ty
%type <fieldList> tyfields tail_tyfields
%type <field> tyfield
%type <paramList> ty_param ty_param_tail

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE LET IN END OF 
  NIL
  FUNCTION VAR TYPE FUNC_RETURN 

%start program

%%

program:  exp                           { absyn_root = $1; }
exp:  lvalue ASSIGN exp                 {$$ = A_AssignExp(EM_tokPos, $1, $3); } 
   |  exp_or                            {$$ = $1;} 
   ; 

exp_or:  exp_and exp_or_rec             {$$ = A_ExpExp($1, $2);}  
exp_or_rec:  OR exp_and exp_or_rec      {$$ = A_IfExp2(EM_tokPos, $<exp>0, A_IntExp(EM_tokPos, 1), $2, $3);} 
          |                             {$$ = NULL;} 
          ;  

exp_and:  exp_compa exp_and_rec         {$$ = A_ExpExp($1, $2);} 
exp_and_rec:  AND exp_compa exp_and_rec {$$ = A_IfExp2(EM_tokPos, $<exp>0, A_IntExp(EM_tokPos, 0), $2, $3);} 
           |                            {$$ = NULL;} 
           ; 

exp_compa: exp_add EQ  exp_add          {$$ = A_OpExp(EM_tokPos, A_eqOp, $1, $3);} 
         | exp_add NEQ exp_add          {$$ = A_OpExp(EM_tokPos, A_neqOp, $1, $3);} 
         | exp_add GT exp_add           {$$ = A_OpExp(EM_tokPos, A_gtOp, $1, $3);} 
         | exp_add GE exp_add           {$$ = A_OpExp(EM_tokPos, A_geOp, $1, $3);} 
         | exp_add LT exp_add           {$$ = A_OpExp(EM_tokPos, A_ltOp, $1, $3);} 
         | exp_add LE exp_add           {$$ = A_OpExp(EM_tokPos, A_leOp, $1, $3);}
         | exp_add                      {$$ = $1;} 
         ; 


exp_add:  exp_mul exp_add_rec             {$$ = A_ExpExp($1, $2);}  

exp_add_rec:  PLUS exp_mul exp_add_rec    {$$ = A_OpExp2(EM_tokPos, A_plusOp, $<exp>0, $2, $3);} 
           |  MINUS exp_mul exp_add_rec   {$$ = A_OpExp2(EM_tokPos, A_minusOp, $<exp>0, $2, $3);} 
           |                              {$$ = NULL;} 
           ; 
exp_mul:  exp_nega exp_mul_rec            {$$ = A_ExpExp($1, $2);}

exp_mul_rec:  TIMES exp_nega exp_mul_rec  { $$ = A_OpExp2(EM_tokPos, A_timesOp, $<exp>0, $2, $3); }
           |  DIVIDE exp_nega exp_mul_rec { $$ = A_OpExp2(EM_tokPos, A_divideOp, $<exp>0, $2, $3); } 
           |                              {$$ = NULL;} 
           ; 

exp_nega:  MINUS exp_nega                  {$$ = A_OpExp(EM_tokPos, A_minusOp, A_IntExp(0, 0), $2);} 
        |  exp_seq                        {$$ = $1;} 
        ; 
exp_seq:  LPAREN expseq RPAREN            {$$ = A_SeqExp(EM_tokPos, $2);} 
       |  factor                          {$$ = $1;} 
       ; 
factor:  NIL                              { $$ = A_NilExp(EM_tokPos); } 
      |  INT                              { $$ = A_IntExp(EM_tokPos, yylval.ival); } 
      |  STRING                           { $$ = A_StringExp(EM_tokPos, yylval.sval); } 
      |  LET decs IN expseq END           { $$ = A_LetExp(EM_tokPos, $2, $4); }
      |  ID LBRACK exp RBRACK OF exp      { $$ = A_ArrayExp(EM_tokPos, S_Symbol($1), $3, $6); }
      |  IF exp THEN exp ELSE exp         { $$ = A_IfExp(EM_tokPos, $2, $4, $6); }
      |  IF exp THEN exp                  { $$ = A_IfExp(EM_tokPos, $2, $4, NULL); }
      |  lvalue                           { $$ = A_VarExp(EM_tokPos, $1); }
      |  funcall                          {$$ = $1;} 
      |  rec_creation                     {$$ = $1;} 
      ;
expseq: exp                   { $$ = A_ExpList($1, NULL); }
      |                       { $$ = NULL; }
      ;
funcall:  ID LPAREN RPAREN          { $$ = A_CallExp(EM_tokPos, S_Symbol($1), NULL); }
       |  ID LPAREN argseq RPAREN   { $$ = A_CallExp(EM_tokPos, S_Symbol($1), $3); }
       ;
argseq: exp COMMA argseq  { $$ = A_ExpList($1, $3); }
      | exp               { $$ = A_ExpList($1, NULL); }
      ;
rec_creation: ID LBRACE RBRACE            { $$ = A_RecordExp(EM_tokPos, S_Symbol($1), NULL); }
            | ID LBRACE efield_seq RBRACE { $$ = A_RecordExp(EM_tokPos, S_Symbol($1), $3); }
            ;
efield_seq: efield COMMA efield_seq { $$ = A_EfieldList($1, $3); }
         |  efield                  { $$ = A_EfieldList($1, NULL); }
         ;
efield:  ID EQ exp  { $$ = A_Efield(S_Symbol($1), $3); }
     ;

decs: dec decs  { $$ = A_DecList($1, $2); }
    |           { $$ = NULL; }
    ;
dec:  tydec   { $$ = $1; }
   |  vardec  { $$ = $1; }
   |  fundec  { $$ = $1; }
   ;

tydec:  TYPE ID EQ ty { $$ = A_TypeDec(EM_tokPos, S_Symbol($2), $4); }
     ;
ty: ID                      { $$ = A_NameTy(S_Symbol($1)); }
  | LBRACE tyfields RBRACE  { $$ = A_RecordTy($2); }
  | ARRAY OF ID             { $$ = A_ArrayTy(S_Symbol($3)); }
  | func_ty                 { $$ = $1; }
  ;

func_ty: LPAREN ty_param RPAREN FUNC_RETURN ID           { $$ = A_FuncTySimpleTypeReturn($2,A_ParameterList(EM_tokPos, S_Symbol($5), NULL)); }
       | LPAREN RPAREN FUNC_RETURN ID                    { $$ = A_FuncTySimpleTypeReturn(NULL, A_ParameterList(EM_tokPos, S_Symbol($4), NULL)); }
       | ID FUNC_RETURN ID                               { $$ = A_FuncTySimpleTypeReturn(A_ParameterList(EM_tokPos, S_Symbol($1), NULL), A_ParameterList(EM_tokPos, S_Symbol($3), NULL)); }
       | LPAREN ty_param RPAREN FUNC_RETURN func_ty      { $$ = A_FuncTyFuncReturn($2,$5); }
       | LPAREN RPAREN FUNC_RETURN func_ty               { $$ = A_FuncTyFuncReturn(NULL,$4); }
       | ID FUNC_RETURN func_ty                          { $$ = A_FuncTyFuncReturn(A_ParameterList(EM_tokPos, S_Symbol($1), NULL), $3); }
       ;

ty_param: ID ty_param_tail  { $$ = A_ParameterList(EM_tokPos, S_Symbol($1), $2); }
        ;

ty_param_tail: COMMA ID ty_param_tail { $$ = A_ParameterList(EM_tokPos, S_Symbol($2), $3); }
             |                        { $$ = NULL; }
             ;

tyfields: tyfield tail_tyfields { $$ = A_FieldList($1, $2); }
        |                       { $$ = NULL; }
        ;

tail_tyfields:  COMMA tyfield tail_tyfields { $$ = A_FieldList($2, $3); }
             |                              { $$ = NULL; }
             ;
tyfield:  ID COLON ID { $$ = A_Field(EM_tokPos, S_Symbol($1), S_Symbol($3)); }
       ;

vardec: VAR ID ASSIGN exp           { $$ = A_VarDec(EM_tokPos, S_Symbol($2), NULL, $4); }
      | VAR ID COLON ID ASSIGN exp  { $$ = A_VarDec(EM_tokPos, S_Symbol($2), S_Symbol($4), $6); }
      ;

fundec: FUNCTION ID LPAREN tyfields RPAREN EQ exp           {$$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, NULL, $7); }
      | FUNCTION ID LPAREN tyfields RPAREN COLON ID EQ exp  {$$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, S_Symbol($7), $9); }
      ;

lvalue: ID            { $$ = A_SimpleVar(EM_tokPos, S_Symbol($1)); }
      | ID DOT ID     { $$ = A_FieldVar(EM_tokPos, A_SimpleVar(EM_tokPos, S_Symbol($1)), S_Symbol($3)); }
      | lvalue DOT ID { $$ = A_FieldVar(EM_tokPos, $1, S_Symbol($3)); }
      | ID LBRACK exp RBRACK      { $$ = A_SubscriptVar(EM_tokPos, A_SimpleVar(EM_tokPos, S_Symbol($1)), $3); }
      | lvalue LBRACK exp RBRACK  { $$ = A_SubscriptVar(EM_tokPos, $1, $3); }
      ;
