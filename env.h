#ifndef __ENV_H__
#define __ENV_H__

#include "types.h"
#include "symbol.h"

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

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

/* initialize the type bindings:
 * map int to Ty_int and string to Ty_string
 */
S_table E_base_tenv(void);

/* initialize the variable bindings:
 * for predefined functions
 */
S_table E_base_venv(void);

#endif
