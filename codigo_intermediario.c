#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analise_semantica.h"
#include "codigo_intermediario.h"
#include "util.h"
#include "symbol.h"

#define MAX_CHARS 20


S_table vf_table;
S_table ty_table;
S_table uni_table;
S_table invoke_table;
S_table var_table;

string_list globlalStrings;

int tempCount = 0;

void getActualTempCount(char **count){
	char *temp = "%%temp%d\0";
	*count = malloc(MAX_CHARS);
	sprintf(*count, temp, tempCount++);
}

void getActualGlobalTempCount(char **count){
	char *temp = "@global%d\0";
	*count = malloc(MAX_CHARS);
	sprintf(*count, temp, tempCount++);
}

void getLabelCount(char **count){
	char *temp = "l%d\0";
	*count = malloc(MAX_CHARS);
	sprintf(*count, temp, tempCount++);
}

void geraCodigoIntermediario(string filename, A_exp e)
{
  filename = remove_ext(filename, '.', '/');
  strcat(filename, ".ll");
  
  FILE* arquive = fopen(filename, "w+");
  vf_table = initializeVariablesAndFunctionsTable();
  ty_table = initializeTypesTable();
  uni_table = initializeUnicodeTable();
  invoke_table = S_empty();
  var_table = S_empty();
  globlalStrings = stringList("", NULL);
  evaluateExp(arquive, e, 0);
  fclose(arquive);
}

S_table initializeUnicodeTable(){
	S_table t = S_empty();
  	S_enter(
    	t,
    	S_Symbol("\n"),
    	"\\0A"
  	);

  	S_enter(
    	t,
    	S_Symbol("\\0"),
    	"\\00"
  	);
  	return t;
}


string_list stringList(char *head, string_list tail)
{
  string_list p = (string_list) checked_malloc(sizeof(*p));
  p->head=head;
  p->tail=tail;
  return p;
}

char *evaluateExp(FILE* arquive, A_exp e, int level)
{ 

	char *integer;
	switch (e->kind) {
		case A_varExp:
			return evaluateVar(arquive, e->u.var, level);
			break;
		case A_nilExp:
			indent(arquive, level);
			fprintf(arquive, "null");
			return "";
			break;
		case A_intExp:
			integer = malloc(20);
			sprintf(integer, "i32 %d", e->u.intt);
			return integer;
			break;
		case A_stringExp:
			return evaluateStringExp(arquive, e->u.stringg, level);
			break;
		case A_callExp:
			return evaluateCallExp(arquive, e, level);
			break;
		case A_opExp:
			return evaluateOpExp(arquive, e, level);	
			break;
		case A_recordExp:
			return evaluateRecordExp(arquive, e, level);
			break;
		case A_seqExp:
			return evaluateSeqExp(arquive, e->u.seq, level);
			break;
		case A_assignExp:
			return evaluateAssignExp(arquive, e, level);
			break;
		case A_ifExp:
			return evaluateIfExp(arquive, e, level);
			break;
    	case A_arrayExp:
    		return evaluateArrayExp(arquive, e, level);
    		break;
		case A_expExp:
			return evaluateExpExp(arquive, e, level);
			break;
		case A_letExp:
      		return evaluateLetExp(arquive, e, level);
      		break;
  }
}

char* subString(const char* input, int offset, int len, char* dest)
{
  int input_len = strlen (input);
  if (offset + len > input_len)
  {
     return NULL;
  }

  strncpy (dest, input + offset, len);
  dest[len] = '\0';
  return dest;
}

char * evaluateStringExp(FILE* arquive, string str, int level)
{
	char *stringTemp;
	char *temp = malloc(1000);
	char *temp2 = malloc(1000);
	char *paramString = malloc(strlen(str));

	subString(str, 1, strlen(str) - 2, paramString);

	char *tempFormat = "%s";
	getActualGlobalTempCount(&stringTemp);

	char *escapedString = malloc(strlen(paramString) + 2);

	int i = 0;
	for(; i < strlen(paramString); i++){
		sprintf(temp2, "%c", paramString[i]);
		if(S_look(uni_table, S_Symbol(temp2)) != NULL){
			sprintf(temp, tempFormat, (char *) S_look(uni_table, S_Symbol(temp2)));
			strcat(escapedString, temp);
		}else{
			sprintf(temp, tempFormat, temp2);
			strcat(escapedString, temp);
		}
	}

	sprintf(temp, tempFormat, (char *) S_look(uni_table, S_Symbol("\\0")));
	strcat(escapedString, temp);

	char *globalString = malloc(1000);
	strcat(globalString, stringTemp);
	strcat(globalString, " = private unnamed_addr constant");

	int size = strlen(paramString) + 1;

	tempFormat = "[%d x i8] c\"%s\"";
	sprintf(temp, tempFormat, size, escapedString);
	strcat(globalString, temp);
	globlalStrings = stringList(globalString, globlalStrings);

	tempFormat = "[%d x i8]";
	sprintf(temp2, tempFormat, size);
	
	tempFormat = "i8* getelementptr inbounds (%s* %s, i32 0, i32 0)";
	sprintf(temp, tempFormat, temp2, stringTemp);

	S_enter(invoke_table, S_Symbol(stringTemp), temp);
	
	return stringTemp;
}

char *evaluateCallExp(FILE* arquive, A_exp e, int level){
	E_enventry ee = S_look(vf_table, e->u.call.func);
	
	char *functionCall = malloc(1000);
	switch(ee->u.fun.result->kind){
		case Ty_int:{
			sprintf(functionCall, "%s", "i32 ");
			break;
		}

		case Ty_string:{
			sprintf(functionCall, "%s", "i8* ");
			break;
		}
	}

	if((strcmp(S_name(e->u.call.func),"print") == 0) || (strcmp(S_name(e->u.call.func),"printint") == 0)){
		globlalStrings = stringList("declare i32 @printf(i8*, ...)", globlalStrings);
		strcat(functionCall, "(i8*, ...)* @printf");
	}else{
		strcat(functionCall, "@");
		strcat(functionCall, S_name(e->u.call.func));
	}

	char *printIntStr;
	if(strcmp(S_name(e->u.call.func),"printint") == 0){
		printIntStr = evaluateStringExp(arquive, "\"%d\n\"", level);
	}

	strcat(functionCall, "(");

	char *tempArg;
	char *returns;
	char *temp = malloc(100);
	char *instruction = malloc(1000);
	char *params = malloc(1000);
	char *returnTy;
	int argKind;
	Ty_ty argTy;
	A_expList args = e->u.call.args;

	if(strcmp(S_name(e->u.call.func),"printint") == 0){
		strcat(params, S_look(invoke_table, S_Symbol(printIntStr)));
		strcat(params, ", ");
	}

	for (; args; args = args->tail) {
		getActualTempCount(&tempArg);

		argTy = analisaExp(vf_table, ty_table, args->head).ty;
		argKind = getRealKind(args->head);
		returns = evaluateExp(arquive, args->head, level+1);

		switch(argTy->kind){
			case Ty_int: returnTy = "i32 "; break;
			case Ty_string: returnTy = "i8* "; break;
		}

		switch(argKind){
			case A_intExp:{
				strcat(params, returns);
				break;
			}
			case A_stringExp:{
				strcat(params, S_look(invoke_table, S_Symbol(returns)));
				break;
			}
			case A_varExp:
			{
				sprintf(temp, "%s = load %s\n", tempArg, returns);
				strcat(instruction, temp);
				strcat(params, returnTy);
				strcat(params, tempArg);
				break;
			}
			case A_opExp: {
				tempArg = returns;
				strcat(params, returnTy);
				strcat(params, tempArg);
				break;
			}

			case A_callExp:{
				sprintf(temp, "%s = call %s\n", tempArg, returns);
				strcat(instruction, temp);
				strcat(params, returnTy);
				strcat(params, tempArg);
			}

		}

		if(args->tail){
			strcat(params, ", ");
		}		
	}

	sprintf(temp, "%s)", params);
	strcat(functionCall, temp);
	fprintf(arquive, "%s\n", instruction);

	if(level == 1){
		getActualTempCount(&tempArg);
		fprintf(arquive, "%s = call ", tempArg);
		fprintf(arquive, "%s\n",functionCall);
	}
	return functionCall;
}

char * evaluateOpExp(FILE* arquive, A_exp e, int level){
	analisaExp(vf_table, ty_table, e->u.op.left);
  	analisaExp(vf_table, ty_table, e->u.op.right);

  	A_exp expRight;
  	if(e->right){
  		expRight = e->right;
  	}else{
  		expRight = e->u.op.right;
  	}

  	char *left;
  	getActualTempCount(&left);
	char *right;
  	getActualTempCount(&right);
  	char *returns;

  	char *tempFormat = "%s = alloca i32\n";
  	char *temp = malloc(100);
  	char *instruction = malloc(1000);

	if(getRealKind(e->u.op.left) != A_callExp){
		sprintf(temp, tempFormat, left);  		
		fprintf(arquive, "%s\n", temp);
	}

	if(getRealKind(expRight) != A_callExp){
  		sprintf(temp, tempFormat, right);
  		fprintf(arquive, "%s\n", temp);
	}

	if(getRealKind(e->u.op.left) == A_callExp){
		tempFormat = "%s = call ";
		sprintf(temp, tempFormat, left);
	}else if(getRealKind(e->u.op.left) != A_opExp){
		strcpy(temp,"store ");
	}
	
	returns = evaluateExp(arquive, e->u.op.left, level);
	
	if(getRealKind(e->u.op.left) != A_opExp){
		strcat(instruction, temp);
		fprintf(arquive, "%s", instruction);
		fprintf(arquive, "%s", returns);
	}


	if(getRealKind(e->u.op.left) != A_callExp && getRealKind(e->u.op.left) != A_opExp){
		tempFormat = ", i32* %s\n";
		sprintf(temp, tempFormat, left);
		fprintf(arquive, "%s", temp);
	}else if(getRealKind(e->u.op.left) == A_opExp){
		left = returns;
	}

	if(getRealKind(expRight) == A_callExp){
		tempFormat = "%s = call ";
		sprintf(temp, tempFormat, right);
	}else if(getRealKind(expRight) != A_opExp){
		strcpy(temp,"store ");
	}

	returns = evaluateExp(arquive, expRight, level);
	
	if(getRealKind(expRight) != A_opExp){
		fprintf(arquive, "%s", temp);
		fprintf(arquive, "%s", returns);
	}
	
	if(getRealKind(expRight) != A_callExp && getRealKind(expRight) != A_opExp){
		tempFormat = ", i32* %s\n";
		sprintf(temp, tempFormat, right);
		fprintf(arquive, "%s", temp);
	}else if(getRealKind(expRight) == A_opExp){
		right = returns;
	}

	fprintf(arquive, "\n");

	char *loadLeft;
	char *loadRight;

	getActualTempCount(&loadRight);
	getActualTempCount(&loadLeft);

	if(isPrintLoad(e->u.op.left)){
		tempFormat = "%s = load i32* %s\n";
		sprintf(temp, tempFormat, loadLeft, left);
		fprintf(arquive, "%s", temp);
	}else{
		loadLeft = left;
	}

	if(isPrintLoad(expRight)){
		tempFormat = "%s = load i32* %s\n";
		sprintf(temp, tempFormat, loadRight, right);
		fprintf(arquive, "%s", temp);
	}else{
		loadRight = right;
	}

	char *opeation;
	switch (e->u.op.oper) {
	    case A_plusOp:
	    	opeation = "add";
	    	break;
	    case A_minusOp:
	    	opeation = "sub";
	    	break;
	    case A_timesOp:
	    	opeation = "mul";
	    	break;
	    case A_divideOp:
	    	opeation = "sdiv";
	    	break;
	    case A_eqOp:
	    	opeation = "icmp eq";
	    	break;
	    case A_neqOp:
	    	opeation = "icmp ne";
	    	break;
	    case A_ltOp:
	    	opeation = "icmp slt";
	    	break;
	    case A_leOp:
	    	opeation = "icmp sle";
	    	break;
	    case A_gtOp:
	    	opeation = "icmp sgt";
	    	break;
	    case A_geOp:
	    	opeation = "icmp sge";
	    	break;
	}

	char *opResult;
	getActualTempCount(&opResult);
	tempFormat = "%s = %s i32 %s, %s";
	sprintf(temp, tempFormat, opResult, opeation, loadLeft, loadRight);
	fprintf(arquive, "%s\n", temp);

	return opResult;
}

int isPrintLoad(A_exp e){
	int theKind = getRealKind(e);
	return (theKind != A_callExp && theKind != A_opExp);
}

char *evaluateRecordExp(FILE* arquive, A_exp e, int level){
	Ty_ty recordType = S_look(ty_table, e->u.record.typ);
	A_efieldList efl = e->u.record.fields;
  	Ty_fieldList tfl = recordType->u.record;

	while (efl && tfl) {
		if (efl->head->name != tfl->head->name) {
			analisaExp(vf_table, ty_table, efl->head->exp);
			efl = efl->tail;
			tfl = tfl->tail;
		}
	}
}

char *evaluateAssignExp(FILE* arquive, A_exp e, int level){
	transVar(vf_table, ty_table, e->u.assign.var);
  	analisaExp(vf_table, ty_table, e->u.assign.exp);
}

char *evaluateIfExp(FILE* arquive, A_exp e, int level){
	analisaExp(vf_table, ty_table, e->u.iff.test);
	analisaExp(vf_table, ty_table, e->u.iff.then);

	if (e->u.iff.elsee) {
	    struct expty elsee = analisaExp(vf_table, ty_table, e->u.iff.elsee);
	}

	char *opResult = evaluateExp(arquive, e->u.iff.test, level);
	
	char *instruction = malloc(1000);
	char *temp = malloc(100);
	char *thenLabel;
	char *endLabel;
	char *elseLabel;
	char *secondLabel;

	getLabelCount(&endLabel);
	if(e->u.iff.elsee){
		getLabelCount(&elseLabel);
		secondLabel = elseLabel;
	}else{
		secondLabel = endLabel;
	}

	char *resultTemp = NULL;
	getActualTempCount(&resultTemp);
	fprintf(arquive, "%s = alloca i32\n", resultTemp);

	getLabelCount(&thenLabel);

	sprintf(temp, "br i1 %s, label %%%s, label %%%s", opResult, thenLabel, secondLabel);
	strcat(instruction, temp);

	fprintf(arquive, "%s\n", instruction);
	fprintf(arquive, "%s:\n", thenLabel);


	char * ifResult = NULL;
	ifResult = evaluateExp(arquive, e->u.iff.then, level + 1);

	if(ifResult != NULL){
		
		if(getRealKind(e->u.iff.then) == A_intExp){
			sprintf(temp, "store %s, i32* %s", ifResult, resultTemp);
		}else if(getRealKind(e->u.iff.then) == A_opExp){
			sprintf(temp, "store i32 %s, i32* %s", ifResult, resultTemp);
		}else{
			sprintf(temp, "%s = load i32* %s", resultTemp, ifResult);	
		}

		
		fprintf(arquive, "%s\n", temp);
	}
	
	fprintf(arquive, "br label %%%s\n", endLabel);

	char *elseResult = NULL;
	if(e->u.iff.elsee){
		fprintf(arquive, "%s:\n", elseLabel);
		elseResult = evaluateExp(arquive, e->u.iff.elsee, level + 1);
	}

	if(elseResult != NULL){
		if(getRealKind(e->u.iff.elsee) == A_intExp){
			sprintf(temp, "store %s, i32* %s", elseResult, resultTemp);
		}else if(getRealKind(e->u.iff.elsee) == A_opExp){
			sprintf(temp, "store i32 %s, i32* %s", elseResult, resultTemp);
		}else{
			sprintf(temp, "%s = load i32* %s", resultTemp, elseResult);	
		}

		fprintf(arquive, "%s\n", temp);
		fprintf(arquive, "br label %%%s\n", endLabel);
	}

	fprintf(arquive, "%s:\n", endLabel);

	return resultTemp;
}

char *evaluateArrayExp(FILE* arquive, A_exp e, int level){
	analisaExp(vf_table, ty_table, e->u.array.size);
	analisaExp(vf_table, ty_table, e->u.array.init);
}

char *evaluateVar(FILE* arquive, A_var v, int level){
	switch(v->kind) {
    	case A_simpleVar: {
    		return S_look(var_table, v->u.simple);
    	}

    	case A_fieldVar: {
    	}
    }

}

char *evaluateLetExp(FILE* arquive, A_exp e, int level){
	A_decList d;
	S_beginScope(vf_table);
  	S_beginScope(ty_table);
  
  	for (d = e->u.let.decs; d; d = d->tail) {
    	transDec(vf_table, ty_table, d->head, d->tail);
  	}

	for (d = e->u.let.decs; d; d = d->tail) {
		evaluateDec(arquive, d->head, level);
	}

	if(level == 0){
		fprintf(arquive, "define i32 @main() {\n");
	}

	transSeqExp(vf_table, ty_table, e->u.let.body);
	evaluateSeqExp(arquive, e->u.let.body, level + 1);

	if(level == 0){
		fprintf(arquive, "\nret i32 0\n}\n");
		printGlobalStringList(arquive);
	}

	S_endScope(vf_table);
 	S_endScope(ty_table);
}

void printGlobalStringList(FILE *arquive){
	while(globlalStrings){
		fprintf(arquive, "%s\n", globlalStrings->head);
		globlalStrings = globlalStrings->tail;
	}
}

char *evaluateDec(FILE* arquive, A_dec d, int level){

	switch(d->kind) {
    	case A_functionDec:
      		return evaluateFuncDec(arquive, d, level);
      		break;

      	case A_varDec:
      		return evaluateVarDec(arquive, d, level);
      		break;
    }
}

char *evaluateVarDec(FILE* arquive, A_dec d, int level){
	
	char *label = "%";
	if(level == 0){
		label = "@";
	}

	Ty_ty expected_ty = NULL;
  	if(d->u.var.typ){
    	expected_ty = S_look(ty_table, d->u.var.typ);
  	}else{
    	expected_ty = getTypeForUntypedVar(vf_table,ty_table,d->u.var.init);
  	}

  	char *returns;
  	char *temp = malloc(100);
  	char *varName = malloc(strlen(S_name(d->u.var.var)) + strlen(label));
  	strcat(varName, label);
  	strcat(varName, S_name(d->u.var.var));
  	char *varNameWithType = malloc(strlen(varName) + 2);

  	char *instruction = malloc(1000);
  	strcpy(instruction, varName);
  	char *tempVar;
  	returns = evaluateExp(arquive, d->u.var.init, level);
  	switch(expected_ty->kind){
  		case Ty_int: {
  			if(strcmp(label, "%") == 0){
	  			int kind = getRealKind(d->u.var.init);
	  			strcat(instruction, " = alloca i32\n");
	  			if(kind == A_callExp){
	  				getActualTempCount(&tempVar);
	  				sprintf(temp, "%s = %s\n", tempVar, returns);
	  				strcat(instruction, temp);
	  				sprintf(temp, "store i32 %s, i32* %s\n", tempVar, varName);
	  			}else{
	  				sprintf(temp, "store %s, i32* %s\n", returns, varName);
	  				strcat(instruction, temp);
	  			}
  			}else{
  				sprintf(temp, " = global %s", returns);
  				strcat(instruction, temp);
  			}

  			sprintf(varNameWithType, "i32 %s", varName);
  			break;
  		}
  		case Ty_string:{
  			break;
  		}
  	}

  	fprintf(arquive, "%s\n", instruction);
  	S_enter(var_table, d->u.var.var, varNameWithType);

  	return varName;
}

char *evaluateFuncDec(FILE* arquive, A_dec d, int level){
	Ty_ty resultTy;
  	if (d->u.function.result)
    	resultTy = S_look(ty_table, d->u.function.result);
  	else
    	resultTy = Ty_Void();

    switch(resultTy->kind){
		case Ty_int:{
			fprintf(arquive, "%s", "define i32 ");
			break;
		}

		case Ty_string:{
			fprintf(arquive, "%s", "define i8* ");
			break;
		}
	}
    
    fprintf(arquive, "@%s ", S_name(d->u.function.name));

    char * params;
    getParams(d->u.function.params, &params);

    if(strlen(params) > 0){
    	fprintf(arquive, "(%s)", params);
    }else{
    	fprintf(arquive, "()");	
    }

    Ty_tyList formalTys = makeFormalTyList(ty_table, d->u.function.params);

    S_beginScope(vf_table);
    S_beginScope(var_table);
	A_fieldList fl = d->u.function.params;
	Ty_tyList tl = formalTys;
	for (; fl && tl; fl = fl->tail, tl = tl->tail) {
		S_enter(vf_table, fl->head->name, E_VarEntry(tl->head));
	}

	fl = d->u.function.params;
	tl = formalTys;
	
	setVarsFunDec(fl, tl);

	fprintf(arquive, " {\n");
	char *returns = NULL;
	returns = evaluateExp(arquive, d->u.function.body, level + 1);

	if(getRealKind(d->u.function.body) == A_intExp){
		if(returns != NULL){
			fprintf(arquive, "ret %s\n", returns);
		}
	}else{
		char *returnLabel;
		getActualTempCount(&returnLabel);
		switch(resultTy->kind){
			case Ty_int:{
				fprintf(arquive, "%s = load i32* %s\n", returnLabel, returns);
				fprintf(arquive, "ret %s %s\n", "i32", returnLabel);
				break;
			}

			case Ty_string:{
				fprintf(arquive, "%s = load i8* %s\n", returnLabel, returns);
				fprintf(arquive, "ret %s %s\n", "i8*", returnLabel);
				break;
			}
		}
	}

	S_endScope(vf_table);
	S_endScope(var_table);
	fprintf(arquive, "\n}\n");

}

void setVarsFunDec(A_fieldList fl, Ty_tyList tl){
	char *varNameWithType;
	for (; fl && tl; fl = fl->tail, tl = tl->tail) {

		varNameWithType = malloc(100);
		switch(tl->head->kind){
			case Ty_int:{
				sprintf(varNameWithType, "i32 %%%s", S_name(fl->head->name));
				break;
			}

			case Ty_string:{
				sprintf(varNameWithType, "i8* %%%s", S_name(fl->head->name));
				break;
			}
		}

		S_enter(var_table, fl->head->name, varNameWithType);
	}
}

void getParams(A_fieldList fieldList, char **params){
	char *temp = "%s %%%s\0";
	char *temp2 = malloc(100);
	*params = malloc(10000);

	A_fieldList fl = fieldList;
	Ty_tyList tl = NULL;
	Ty_tyList ret = NULL;
	Ty_ty ty;

	for (; fl; fl = fl->tail) {
		ty = S_look(ty_table, fl->head->typ);
		
		switch(ty->kind){
			case Ty_int:{
				sprintf(temp2, temp, "i32", S_name(fl->head->name));
				break;
			}

			case Ty_string:{
				sprintf(temp2, temp, "i8*", S_name(fl->head->name));
				break;
			}
		}

		strcat(*params, temp2);
		if(fl->tail){
			strcat(*params, ", ");
		}
	}
}

char *evaluateSeqExp(FILE* arquive, A_expList el, int level)
{
  while (el->tail) {
    evaluateExp(arquive, el->head, level);
    el = el->tail;
  }

  evaluateExp(arquive, el->head, level);
}

char *evaluateExpExp(FILE* arquive, A_exp e, int level){
	if(e->right){
		return evaluateExp(arquive, e->right, level);
    }else{
    	return evaluateExp(arquive, e->u.expp, level);
  	}
}

int getRealKind(A_exp e){
	if(e->kind == A_expExp){
		if(e->right){
			return getRealKind(e->right);
	    }else{
	    	return getRealKind(e->u.expp);
	  	}
	}

	return e->kind;
}