#include "analise_semantica.h"
#include "util.h"
#include "symbol.h"
void SEM_transProg(A_exp e, int *isErrors)
{
  S_table vf_table = initializeVariablesAndFunctionsTable();
  S_table ty_table = initializeTypesTable();
  analisaExp(vf_table, ty_table, e);
  *isErrors = EM_getIsErrors();
}

struct expty ExpTy(Tr_exp exp, Ty_ty ty)
{
  struct expty e;
  e.exp = exp;
  e.ty = ty;
  return e;
}

struct expty analisaExp(S_table vf_table, S_table ty_table, A_exp e)
{
  switch (e->kind) {
    case A_varExp:
      return transVar(vf_table, ty_table, e->u.var);

    case A_nilExp:
      return ExpTy(NULL, Ty_Nil());

    case A_intExp:
      return ExpTy(NULL, Ty_Int());

    case A_stringExp:
      return ExpTy(NULL, Ty_String());

    case A_callExp:
      return transCallExp(vf_table, ty_table, e);

    case A_opExp:
      return transOpExp(vf_table, ty_table, e);

    case A_recordExp:
      return transRecordExp(vf_table, ty_table, e);

    case A_seqExp:
      return transSeqExp(vf_table, ty_table, e->u.seq);

    case A_assignExp:
      return transAssignExp(vf_table, ty_table, e);

    case A_ifExp:
      return transIfExp(vf_table, ty_table, e);

    case A_letExp:
      return transLetExp(vf_table, ty_table, e);

    case A_arrayExp:
      return transArrayExp(vf_table, ty_table, e);

    case A_expExp:
      return transExpExp(vf_table, ty_table, e);
  }

  assert(0);
}

struct expty transLetExp(S_table venv, S_table tenv, A_exp e)
{
  struct expty exp;
  A_decList d;
  S_beginScope(venv);
  S_beginScope(tenv);

  for (d = e->u.let.decs; d; d = d->tail) {
    transDec(venv, tenv, d->head, d->tail);
  }

  exp = transSeqExp(venv, tenv, e->u.let.body);

  S_endScope(venv);
  S_endScope(tenv);

  return exp;
}


struct expty transCallExp(S_table vf_table, S_table ty_table, A_exp e)
{
  E_enventry ee = S_look(vf_table, e->u.call.func);

  if (!ee) {
    EM_error(e->pos, "Funcao \"%s\" Indefinida ", S_name(e->u.call.func));
    return ExpTy(NULL, NULL);
  }

  Ty_tyList formals = NULL;
  if(ee->kind == E_varEntry){
    Ty_ty type = ee->u.var.ty;
    if(type->kind == Ty_funcWithFuncReturn || type->kind == Ty_funcWithSimpReturn){
      formals = type->parameter;
    }else{
      EM_error(e->pos, "A chamada de funcao refere-se a uma variavel que não é de um tipo funcao [variavel: %s]", S_name(e->u.call.func));
      return ExpTy(NULL, NULL);
    }
  }else{
    formals = ee->u.fun.formals;
  }

  A_expList args = e->u.call.args;
  for (; formals && args; formals = formals->tail, args = args->tail) {

    if (!Ty_is_compatible(formals->head, analisaExp(vf_table, ty_table, args->head).ty)) {
      EM_error(e->pos, "Os tipos dos argumentos nao sao iguais aos tipos definidos pela funcao");
      return ExpTy(NULL, NULL);
    }
  }

  /* Se a lista de parametros formais já acabou,
  *  porém ainda existem argumentos a serem avaliados
  *  Significa que o numero de parametros passados 
  *  é superior ao numero de parametros da funcao
  */
  if(!formals && args){
    EM_error(e->pos, "O numero de parametros passados na funcao \"%s\" é superior ao numero de parametros definidos na assinatura da funcao", S_name(e->u.call.func));
    return ExpTy(NULL, NULL);
  }

  Ty_ty returns = NULL;
  if(ee->kind == E_varEntry){
    Ty_ty type = ee->u.var.ty;
    if(type->kind == Ty_funcWithFuncReturn || type->kind == Ty_funcWithSimpReturn){
      returns = type->returns;
    }else{
      return ExpTy(NULL, NULL);
    }
  }else{
    returns = ee->u.fun.result;
  }

  return ExpTy(NULL, returns);
}

struct expty transOpExp(S_table vf_table, S_table ty_table, A_exp e)
{
  struct expty left = analisaExp(vf_table, ty_table, e->u.op.left);
  struct expty right = analisaExp(vf_table, ty_table, e->u.op.right);

  switch (e->u.op.oper) {
    case A_plusOp:
    case A_minusOp:
    case A_timesOp:
    case A_divideOp:
      if (left.ty->kind != Ty_int) {
        EM_error(e->u.op.left->pos, "O elemento esquerdo deve ser equivalente a um inteiro");
      }
      if (right.ty->kind != Ty_int) {
        EM_error(e->u.op.right->pos, "O elemento direito deve ser equivalente a um inteiro");
      }
      return ExpTy(NULL, Ty_Int());

    case A_eqOp:
    case A_neqOp:
    case A_ltOp:
    case A_leOp:
    case A_gtOp:
    case A_geOp:
      if (!Ty_is_compatible(left.ty, right.ty)) {
        EM_error(e->pos, "Tipos incompativeis com o tipo do operador");
      }
      return ExpTy(NULL, Ty_Int());
  }

  assert(0);
}

struct expty transRecordExp(S_table vf_table, S_table ty_table, A_exp e)
{
  Ty_ty recordType = S_look(ty_table, e->u.record.typ);
  if (!recordType) {
    EM_error(e->pos, "Registro indefinido %s",
        S_name(e->u.record.typ));
    return ExpTy(NULL, NULL);
  }

  A_efieldList efl = e->u.record.fields;
  Ty_fieldList tfl = recordType->u.record;

  while (efl && tfl) {
    if (efl->head->name != tfl->head->name) {
      EM_error(e->pos, "Erro: Nome do campo = %s diferente do nome do campo definido pelo registro = %s", S_name(efl->head->name), S_name(tfl->head->name));
    }

    Ty_ty assignmentType = analisaExp(vf_table, ty_table, efl->head->exp).ty;

    if (assignmentType != tfl->head->ty) {
      if(assignmentType->kind != Ty_nil){
        EM_error(e->pos, "Erro na definicao de tipo do campo %s", S_name(efl->head->name));
      }
    }

    efl = efl->tail;
    tfl = tfl->tail;
  }
  return ExpTy(NULL, recordType);
}

struct expty transSeqExp(S_table vf_table, S_table ty_table, A_expList el)
{
  while (el->tail) {
    analisaExp(vf_table, ty_table, el->head);
    el = el->tail;
  }

  return analisaExp(vf_table, ty_table, el->head);
}


struct expty transExpExp(S_table vf_table, S_table ty_table,A_exp exp)
{

  if(exp->right){
    return analisaExp(vf_table, ty_table, exp->right);
  }else{
    return analisaExp(vf_table, ty_table, exp->u.expp);
  }

}

struct expty transAssignExp(S_table vf_table, S_table ty_table, A_exp e)
{
  struct expty lv = transVar(vf_table, ty_table, e->u.assign.var);
  struct expty exp = analisaExp(vf_table, ty_table, e->u.assign.exp);
  if (lv.ty && exp.ty && !Ty_is_compatible(lv.ty, exp.ty)) {
    EM_error(e->pos, "atibuicao incompativel com o tipo da variavel");
  }

  return ExpTy(NULL, Ty_Void());
}

struct expty transIfExp(S_table vf_table, S_table ty_table, A_exp e)
{
  struct expty test = analisaExp(vf_table, ty_table, e->u.iff.test);
  struct expty then = analisaExp(vf_table, ty_table, e->u.iff.then);

  if (test.ty != Ty_Int()) {
    EM_error(e->pos, "A condicao do if não retorna um inteiro");
  }

  if (e->u.iff.elsee) {
    struct expty elsee = analisaExp(vf_table, ty_table, e->u.iff.elsee);
    if (!Ty_is_compatible(then.ty, elsee.ty)) {
      EM_error(e->pos, "then e else retornam tipos diferentes");
    }
    return ExpTy(NULL, elsee.ty);
  } else {
    if (then.ty != Ty_Void()) {
      EM_error(e->pos, "Then não pode ser uma expressao com um tipo");
    }
    return ExpTy(NULL, Ty_Void());
  }
}

struct expty transArrayExp(S_table vf_table, S_table ty_table, A_exp e)
{
  struct expty size = analisaExp(vf_table, ty_table, e->u.array.size);
  struct expty init = analisaExp(vf_table, ty_table, e->u.array.init);
  Ty_ty typ = S_look(ty_table, e->u.array.typ);

  if (size.ty != Ty_Int()) {
    EM_error(e->pos, "O tamanho passado para o array nao corresponde a um tipo int");
  }

  if (getPrimitiveType(typ)->kind != Ty_array) {
    EM_error(e->pos, "%s nao corresponde a um tipo array", S_name(e->u.array.typ));
  }

  if (!isTypeEquivalent(init.ty,getPrimitiveType(typ)->u.array)) {
    EM_error(e->pos, "o tipo do elemento passado nao bate com o tipo do array");
  }

  return ExpTy(NULL, typ);
}

struct expty transVar(S_table vf_table, S_table ty_table, A_var v)
{
  switch(v->kind) {
    case A_simpleVar: {

    	// Se for uma simples variavel, verifico se a mesma foi declarada previamente
    	// Ex: charCapturado
  		E_enventry x = S_look(vf_table, v->u.simple);
  		if (x && (x->kind == E_varEntry || x->kind == E_funEntry))
  			return ExpTy(NULL, x->u.var.ty);
  		else {
  			// Se ela nao existir, retorno um erro
  			EM_error(v->pos, "Variavel \"%s\" Indefinida", S_name(v->u.simple));
  			return ExpTy(NULL, NULL);
  		}
    }

    case A_fieldVar: {
      
    	/*
    	*	Se for um nó que representa um acesso a alguma variavel 
    	*	Entao eu faço uma analise na primeira variavel para verificar
    	*	se a mesma existe e se existe o campo que estou tentando acessar 
    	*	nesta varialvel.
    	*	EX: pessoa.nome 
    	*/

  		struct expty et = transVar(vf_table, ty_table, v->u.field.var);

  		if (et.ty && et.ty->kind == Ty_record) {
  			Ty_fieldList fl = et.ty->u.record;
  			for(; fl; fl = fl->tail) {
  		  		if (fl->head->name == v->u.field.sym) {
  		    		return ExpTy(NULL, fl->head->ty);
  		  		}
  			}
        // Se nao existir a variavel de origem e/ou o campo nao existe 
        // Retorno um erro
        EM_error(v->pos, "Variavel \"%s\" Indefinida dentro do registro",S_name(v->u.field.sym));
  		}else if(et.ty && et.ty->kind != Ty_record){
        EM_error(v->pos, "Variavel associada a \"%s\" não é um registro",S_name(v->u.field.sym));
      }else{
    		// Se nao existir a variavel de origem e/ou o campo nao existe 
    		// Retorno um erro
    		EM_error(v->pos, "Variavel \"%s\" Indefinida dentro do registro",S_name(v->u.field.sym));
  		}
      return ExpTy(NULL, NULL);

    }

    case A_subscriptVar: {

    	/*
    	*	Esta derivacao indica o acesso a um array, entao
    	*	devo verificar se o array acessado está declarado
    	*	Ex: pessoas[0]
    	*/
  		struct expty et = transVar(vf_table, ty_table, v->u.field.var);
  		if (et.ty && getPrimitiveType(et.ty)->kind == Ty_array) {
  			return ExpTy(NULL, et.ty->u.array);
  		}else if(et.ty) {
        // Se a Variavel existir, mas não for um array, retorno um erro
        EM_error(v->pos, "Variavel \"%s\" não é um array", S_name(v->u.field.var->u.simple));
      }

  		return ExpTy(NULL, NULL);
    }
  }
}

void transDec(S_table vf_table, S_table ty_table, A_dec d, A_decList tail)
{
  switch(d->kind) {
    case A_functionDec:
      transFuncDec(vf_table, ty_table, d, tail);
      break;
    case A_varDec:
      transVarDec(vf_table, ty_table, d);
      break;
    case A_typeDec:
      transTypeDec(vf_table, ty_table, d, tail);
      break;
  }
}

Ty_tyList makeFormalTyList(S_table ty_table, A_fieldList fieldList)
{
  A_fieldList fl = fieldList;
  Ty_tyList tl = NULL;
  Ty_tyList ret = NULL;
  Ty_ty ty;

  for (; fl; fl = fl->tail) {
    ty = S_look(ty_table, fl->head->typ);
    if (ty) {
      if (tl) {
        tl = tl->tail = Ty_TyList(ty, NULL);
      } else {
        ret = tl = Ty_TyList(ty, tl);
      }
    } else {
      EM_error(fl->head->pos, "Tipo indefinido [tipo = %s]", S_name(fl->head->typ));
    }
  }

  return ret;
}

int look4SomethingWithSameName(string funname, A_decList tail, int kind){
  A_decList t = tail;
  while(t && t->head->kind == kind){
    if(strcmp(funname, S_name(t->head->u.function.name)) == 0){
      return 1;
    }
    t = t->tail;
  }
  return 0;
}

void transFuncDec(S_table vf_table, S_table ty_table, A_dec d, A_decList tail)
{

  if(look4SomethingWithSameName(S_name(d->u.function.name), tail, A_functionDec)){
    EM_error(d->pos, "Não pode existir duas funções consecutivas com o mesmo nome [ funcao: %s]", S_name(d->u.function.name));
    return;
  }

  Ty_ty resultTy;
  if (d->u.function.result)
    resultTy = S_look(ty_table, d->u.function.result);
  else
    resultTy = Ty_Void();

  Ty_tyList formalTys = makeFormalTyList(ty_table, d->u.function.params);
  S_enter(vf_table, d->u.function.name, E_FunEntry(formalTys, resultTy));

  S_beginScope(vf_table);
  A_fieldList fl = d->u.function.params;
  Ty_tyList tl = formalTys;
  for (; fl && tl; fl = fl->tail, tl = tl->tail) {
    S_enter(vf_table, fl->head->name, E_VarEntry(tl->head));
  }

  A_decList funcs = tail;
  A_dec fucaoDaVez = NULL;
  while(funcs && funcs->head->kind == A_functionDec){
    transFuncDec(vf_table, ty_table, funcs->head, funcs->tail);
    funcs = funcs->tail;
  }

  analisaExp(vf_table, ty_table, d->u.function.body);
  S_endScope(vf_table);
}

void transVarDec(S_table vf_table, S_table ty_table, A_dec d)
{
  struct expty e = analisaExp(vf_table, ty_table, d->u.var.init);

  Ty_ty expected_ty = NULL;
  if(d->u.var.typ){
    expected_ty = S_look(ty_table, d->u.var.typ);
  }else{
    expected_ty = getTypeForUntypedVar(vf_table,ty_table,d->u.var.init);
  }

  if (!expected_ty) {
    EM_error(d->pos, "Tipo indefinido ou variavel sem tipo [variavel: %s]", S_name(d->u.var.var));
    return;
  }

  if ((expected_ty->kind != Ty_record && e.ty == Ty_Nil()) 
        || (!isTypeEquivalent(expected_ty, e.ty) && expected_ty->kind != Ty_record)
        || (expected_ty->kind == Ty_record && !Ty_is_compatible(expected_ty, e.ty))) {
    EM_error(d->pos, "Tipo da variavel inconsistente com o tipo do valor atribuido [variavel: %s]",
        S_name(d->u.var.var));
    return;
  }

  if(expected_ty){
    if(expected_ty->kind == Ty_funcWithSimpReturn || expected_ty->kind == Ty_funcWithFuncReturn){
      S_enter(vf_table, d->u.var.var, E_VarEntry(expected_ty));
    }else{
      S_enter(vf_table, d->u.var.var, E_VarEntry(e.ty));
    }
  }
}

void transTypeDec(S_table vf_table, S_table ty_table, A_dec d, A_decList tail)
{

  if(look4SomethingWithSameName(S_name(d->u.type.name), tail, A_typeDec)){
    EM_error(d->pos, "Não pode existir dois tipos consecutivos com o mesmo nome [ Tipo: %s]", S_name(d->u.type.name));
    return;
  }

  Ty_ty ty = transTy(ty_table, d->u.type.name, d->u.type.ty, tail);
  if (ty){
    S_enter(ty_table, d->u.type.name, ty);
  }
  else{
    EM_error(d->pos, "Não foi possivel declarar o tipo: %s", S_name(d->u.type.name));
  }
}

Ty_ty transTy(S_table ty_table, S_symbol ty_name, A_ty t, A_decList tail)
{
  switch(t->kind) {
    case A_nameTy: {
      Ty_ty ty = S_look(ty_table, t->u.name);
      if (ty)
        return Ty_Name(ty);
      else
        return NULL;
    }
    case A_recordTy: {
      A_fieldList a_record = NULL;
      A_field a_field = NULL;
      Ty_fieldList t_record = NULL;
      Ty_fieldList saved_t_record = NULL;
      Ty_ty ty = NULL;

      /* So e permitido um record type referenciar a si mesmo ou 
       * referenciar a tipos conhecidos
       * type: type record = { id : record, id : other_defined_type }
       */
      for (a_record = t->u.record; a_record; a_record = a_record->tail) {
        a_field = a_record->head;
        ty = S_look(ty_table, a_field->typ);
        if (ty_name == a_field->typ || ty) {
          if (t_record) {
            t_record->tail = Ty_FieldList(Ty_Field(a_field->name, ty), NULL);
            t_record = t_record->tail;
          } else {
            t_record = Ty_FieldList(Ty_Field(a_field->name, ty), NULL);
            saved_t_record = t_record;
          }
        } else {
          A_decList types = tail;
          while(types && types->head->kind == A_typeDec && types->head->u.type.name != a_field->typ){
            types = types->tail;
          }
          if(types && types->head->u.type.name != a_field->typ){
            EM_error(a_field->pos, "Tipo indefinido na declaracao do registo: %s", S_name(a_field->typ));
            return NULL;
          }
        }
      }
      
      Ty_ty new_record = Ty_Record(saved_t_record);
      Ty_fieldList tfl = NULL;
      for (tfl = t_record; tfl; tfl = tfl->tail) {
        if (!tfl->head->ty) {
          tfl->head->ty = new_record;
        }
      }
      return new_record;
    }
    case A_arrayTy: {
      Ty_ty ty = S_look(ty_table, t->u.array);
      if (ty)
        return Ty_Array(ty);
      else
        return NULL;
    }
    case A_funcTyFuncReturn: {
      A_paramList parametro = t->parameter;
      Ty_ty ty = NULL;
      Ty_tyList parametrosList = NULL;

      for(; parametro; parametro = parametro->tail){
        ty = S_look(ty_table, parametro->head);
        if(!ty){
          EM_error(parametro->pos, "Tipo indefinido (%s) como parametro", S_name(parametro->head));
          return NULL;
        }
      }

      ty = transTy(ty_table, NULL, t->returns.ty, tail);
      if(!ty){
        //Espera-se que dentro da transTy seja retornado um erro
        //Por isto nao printo um erro neste if
        return NULL;
      }
      
      parametro = t->parameter;
      parametrosList = parseParamListToTyList(ty_table,parametro);

      return Ty_FuncWithFuncReturn(parametrosList, ty);
      
    }
    case A_funcTySymReturn: {

      A_paramList parametro = t->parameter;
      Ty_ty ty = NULL;
      Ty_tyList parametrosList = NULL;

      for(; parametro;parametro = parametro->tail){
        ty = S_look(ty_table, parametro->head);
        if(!ty){
          EM_error(parametro->pos, "Tipo indefinido (%s) como parametro", S_name(parametro->head));
          return NULL;
        }
      }

      ty = S_look(ty_table, t->returns.sym->head);
      if(!ty){
        A_paramList s = t->returns.sym;
        EM_error(s->pos, "Tipo indefinido (%s) como retorno", S_name(s->head));
        return NULL;
      }
      parametro = t->parameter;
      parametrosList = parseParamListToTyList(ty_table,parametro);

      return Ty_FuncWithFuncReturn(parametrosList, ty);

    }
  }
}

Ty_tyList parseParamListToTyList(S_table ty_table,A_paramList parametros){
  if(parametros){
    return Ty_TyList(S_look(ty_table, parametros->head), parseParamListToTyList(ty_table, parametros->tail));
  }
  return NULL;
}

E_enventry E_VarEntry(Ty_ty ty)
{
  E_enventry p = checked_malloc(sizeof(*p));
  p->kind = E_varEntry;
  p->u.var.ty = ty;
  return p;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
  E_enventry p = checked_malloc(sizeof(*p));
  p->kind = E_funEntry;
  p->u.fun.formals = formals;
  p->u.fun.result = result;
  return p;
}

S_table initializeTypesTable(void)
{
  S_table t = S_empty();
  S_enter(t, S_Symbol("int"), Ty_Int());
  S_enter(t, S_Symbol("string"), Ty_String());
  return t;
}

S_table initializeVariablesAndFunctionsTable(void)
{
  S_table t = S_empty();
  S_enter(
    t,
    S_Symbol("print"),
    E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int())
  );
  S_enter(
    t,
    S_Symbol("printint"),
    E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Int())
  );
  S_enter(
    t,
    S_Symbol("flush"),
    E_FunEntry(NULL, Ty_Void())
  );
  S_enter(
    t,
    S_Symbol("getchar"),
    E_FunEntry(NULL, Ty_String())
  );
  S_enter(
    t,
    S_Symbol("ord"),
    E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int())
  );
  S_enter(
    t,
    S_Symbol("chr"),
    E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_String())
  );
  S_enter(
    t,
    S_Symbol("size"),
    E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int())
  );
  S_enter(
    t,
    S_Symbol("substring"),
    E_FunEntry(Ty_TyList(Ty_String(),
                         Ty_TyList(Ty_Int(),
                                   Ty_TyList(Ty_Int(), NULL))),
      Ty_String())
  );
  S_enter(
    t,
    S_Symbol("concat"),
    E_FunEntry(Ty_TyList(Ty_String(),
                         Ty_TyList(Ty_String(), NULL)),
      Ty_String())
  );
  S_enter(
    t,
    S_Symbol("not"),
    E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Int())
  );
  S_enter(
    t,
    S_Symbol("exit"),
    E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Void())
  );
  
  return t;

}

int isTypeEquivalent(Ty_ty t1, Ty_ty t2){
  Ty_ty typeRootOfT1 = t1;
  Ty_ty typeRootOfT2 = t2;    

  while(typeRootOfT1 && typeRootOfT1->kind == Ty_name){
    typeRootOfT1 = typeRootOfT1->u.name;
  }

  while(typeRootOfT2 && typeRootOfT2->kind == Ty_name){
    typeRootOfT2 = typeRootOfT2->u.name;
  }

  return typeRootOfT1 == typeRootOfT2;
}

Ty_ty getTypeForUntypedVar(S_table vf_table, S_table ty_table, A_exp exp){
  switch(exp->kind){
    case A_expExp: {
      if(exp->right){
        return getTypeForUntypedVar(vf_table, ty_table,exp->right);
      }else{
        return getTypeForUntypedVar(vf_table, ty_table, exp->u.expp);
      }
      break;
    }
  }

  struct expty e = analisaExp(vf_table, ty_table, exp);
  return e.ty;
}

Ty_ty getPrimitiveType(Ty_ty t){

  Ty_ty typePrimitive = t;

  while(typePrimitive->kind == Ty_name){
    typePrimitive = typePrimitive->u.name;
  }

  return typePrimitive;
}