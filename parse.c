/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include "util.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"

extern int yyparse(void);
extern A_exp absyn_root;

/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{
  EM_reset(fname);
  if (yyparse() == 0) { /* parsing worked */
    printf("parse successfully!\n");
    return absyn_root;
  }
  else return NULL;
}
