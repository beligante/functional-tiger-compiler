#ifndef __TYPES_H__
#define __TYPES_H__

#include "symbol.h"

typedef struct Ty_ty_ *Ty_ty;
typedef struct Ty_tyList_ *Ty_tyList;
typedef struct Ty_field_ *Ty_field;
typedef struct Ty_fieldList_ *Ty_fieldList;

struct Ty_ty_ {
  enum {
    Ty_record,
    Ty_nil,
    Ty_int,
    Ty_string,
    Ty_array,
    Ty_name,
    Ty_void,
    Ty_funcWithFuncReturn,
    Ty_funcWithSimpReturn
  } kind;
  union {
    Ty_fieldList record;
    Ty_ty array;
    Ty_ty name;
  } u;

  Ty_tyList parameter;
  Ty_ty returns;

};

struct Ty_tyList_ {
  Ty_ty head;
  Ty_tyList tail;
};
struct Ty_field_ {
  S_symbol name; 
  Ty_ty ty;
};
struct Ty_fieldList_ {
  Ty_field head;
  Ty_fieldList tail;
};

Ty_ty Ty_Nil(void);
Ty_ty Ty_Int(void);
Ty_ty Ty_String(void);
Ty_ty Ty_Void(void);

Ty_ty Ty_Record(Ty_fieldList fields);
Ty_ty Ty_Array(Ty_ty ty);
Ty_ty Ty_Name(Ty_ty ty);
Ty_ty Ty_FuncWithFuncReturn(Ty_tyList parameter, Ty_ty returns);
Ty_ty Ty_FuncWithSimpleTypeReturn(Ty_tyList parameter, Ty_ty returns);

Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail);
Ty_field Ty_Field(S_symbol name, Ty_ty ty);
Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail);
int Ty_is_compatible(Ty_ty ty1, Ty_ty ty2);

void Ty_print(Ty_ty t);
void TyList_print(Ty_tyList list);

#endif
