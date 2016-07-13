#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"
#include "codigo_intermediario.h"

int main(int argc, char **argv)
{
  extern int yydebug;
  yydebug = 1;

  if (argc < 2) {
    fprintf(stderr,"Utilize o comando da seguinte forma: compilador.tge filename -options\n");
    exit(1);
  }

  char *filename = argv[1];

  A_exp prg = parse(filename);
  int IS_ERRORS;
  if (prg != NULL)
  {
    printf("\n[ ANALISE SINTATICA ] Completada!! :) \n");
    printf("[ ANALISE SEMANTICA ] Iniciando!\n");
    SEM_transProg(prg, &IS_ERRORS);
    printf("[ ANALISE SEMANTICA ] Completada! :D \n");
  }else{
    printf("[ ANALISE SEMANTICA ] Não entrou! (ಠ_ಠ) \n");
  }

  if(!IS_ERRORS){
    char *treeFileName = malloc(500);
    sprintf(treeFileName, "%s",filename);
    treeFileName = remove_ext(treeFileName, '.', '/');
    strcat(treeFileName, ".arv");
    FILE* treefile = fopen(treeFileName, "w+");
    printf("[ EMISSÃO DA ARVORE ] Iniciando!\n");
    pr_exp(treefile, prg, 0);
    printf("[ EMISSÃO DA ARVORE ] Completada! XD \n");

    char *emit = "-llvm-emit";
    if(argc == 3 && (strcmp(emit, argv[2]) == 0)){
      printf("[ GERACAO DE CODIGO ] Iniciando!\n");
      geraCodigoIntermediario(filename, prg);
      printf("[ GERACAO DE CODGIO ] Completada! (͡° ͜ʖ ͡°)\n");
    }
  }

  return 0;
}