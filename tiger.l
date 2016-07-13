%{
  #include <string.h>
  #include "util.h" 
  #include "absyn.h"
  #include "tiger.tab.h"
  #include "errormsg.h"

  char string_text[6007] = "";
  int  comment_depth = 0;
  int charPos=1;

  void adjust(void);
%}

digit [0-9]
letter [a-zA-Z]
all [0-9a-zA-Z_]
ws [ \t\n]
asc_range ([01][0-9]{2}|2[0-4][0-9]|25[0-5])

%x STRING_S STRING_ML COMMENT
%%

<INITIAL>"\""   {adjust(); BEGIN(STRING_S); string_text[0]= '\0'; strcat(string_text,"\"");}

<INITIAL>"/*"   {adjust(); BEGIN(COMMENT); comment_depth++;}

<INITIAL>[ \t]  {adjust(); continue;}
<INITIAL>"\n"   {adjust(); EM_newline(); continue;}

<INITIAL>"("    {adjust(); return LPAREN;}
<INITIAL>")"    {adjust(); return RPAREN;}
<INITIAL>"["    {adjust(); return LBRACK;}
<INITIAL>"]"    {adjust(); return RBRACK;}
<INITIAL>"{"    {adjust(); return LBRACE;}
<INITIAL>"}"    {adjust(); return RBRACE;}
<INITIAL>":"    {adjust(); return COLON;}
<INITIAL>":="   {adjust(); return ASSIGN;}
<INITIAL>"."    {adjust(); return DOT;}
<INITIAL>","    {adjust(); return COMMA;}
<INITIAL>";"    {adjust(); return SEMICOLON;}
<INITIAL>"*"    {adjust(); return TIMES;;}
<INITIAL>"/"    {adjust(); return DIVIDE;}
<INITIAL>"+"    {adjust(); return PLUS;}
<INITIAL>"-"    {adjust(); return MINUS;}
<INITIAL>"="    {adjust(); return EQ;}
<INITIAL>"<>"   {adjust(); return NEQ;}
<INITIAL>">"    {adjust(); return GT;}
<INITIAL>"<"    {adjust(); return LT;}
<INITIAL>">="   {adjust(); return GE;}
<INITIAL>"<="   {adjust(); return LE;}
<INITIAL>"&"    {adjust(); return AND;}
<INITIAL>"|"    {adjust(); return OR;}
<INITIAL>"->"   {adjust(); return FUNC_RETURN;}

<INITIAL>"array"    {adjust(); return ARRAY;}
<INITIAL>"else"     {adjust(); return ELSE;}
<INITIAL>"end"      {adjust(); return END;}
<INITIAL>"function" {adjust(); return FUNCTION;}
<INITIAL>"if"       {adjust(); return IF;}
<INITIAL>"in"       {adjust(); return IN;}
<INITIAL>"let"      {adjust(); return LET;}
<INITIAL>"nil"      {adjust(); return NIL;}
<INITIAL>"of"       {adjust(); return OF;}
<INITIAL>"then"     {adjust(); return THEN;}
<INITIAL>"type"     {adjust(); return TYPE;;}
<INITIAL>"var"      {adjust(); return VAR;}

<INITIAL>{letter}{all}* {adjust(); yylval.sval=String(yytext); return ID;}

<INITIAL>{digit}+ {adjust(); yylval.ival=atoi(yytext); return INT;}

<INITIAL>.                          {adjust(); EM_error(EM_tokPos,"INVALID INPUT");}
<INITIAL>{digit}+{letter}*          {adjust(); EM_error(EM_tokPos,"INVALID INPUT");}
<INITIAL>{letter}{all}*\.+{digit}*  {adjust(); EM_error(EM_tokPos,"INVALID INPUT");}

<STRING_ML>[ \t]  {adjust(); continue;}
<STRING_ML>"\n"   {adjust(); EM_newline(); continue;}
<STRING_ML>\\     {adjust();  BEGIN(STRING_S);}
<STRING_ML>.      {adjust();  EM_error(EM_tokPos,"INVALID CHAR ENTRE MULTILINE STRING");}

<STRING_S>"\""          {adjust();  BEGIN(INITIAL); strcat(string_text,"\""); yylval.sval=String(string_text); return STRING;}
<STRING_S>"\n"          {adjust(); EM_newline(); BEGIN(INITIAL);  EM_error(EM_tokPos,"STRING NAO FECHADA!");}
<STRING_S>"\\n"         {adjust();  strcat(string_text,"\n"); continue;}
<STRING_S>"\\t"         {adjust();  strcat(string_text,"\t"); continue;}
<STRING_S>"\\\\"        {adjust();  strcat(string_text,"\\"); continue;}
<STRING_S>"\\\""        {adjust();  strcat(string_text,"\""); continue;}
<STRING_S>\\{asc_range} {adjust();  strcat(string_text,yytext); continue;}
<STRING_S>\\[ \t]+      {adjust();  BEGIN(STRING_ML);}
<STRING_S>\\\n          {adjust(); EM_newline(); BEGIN(STRING_ML);}
<STRING_S>\\.           {adjust();  EM_error(EM_tokPos,"INVALID ESCAPE OR ASC CODE");}
<STRING_S>.             {adjust();  strcat(string_text,yytext); continue;}


<COMMENT>.      {adjust();}
<COMMENT>"\n"   {adjust(); EM_newline();}
<COMMENT>"/*"   {adjust();  comment_depth++;}
<COMMENT>"*/"   {adjust();  if (--comment_depth == 0) {BEGIN(INITIAL);}}

%%

int yywrap(void) {
  charPos=1;
  return 1;
}

void adjust(void)
{
  EM_tokPos=charPos;
  charPos+=yyleng;
}