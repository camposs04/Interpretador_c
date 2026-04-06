%{
#include "parserC.tab.h"
#include <stdio.h>
#include <stdlib.h>
%}

%%

"if"        { return IF;}
"else"      { return ELSE;}

"int"       { return INT;}
"float"     { return FLOAT;}
"char"      { return CHAR;}
"bool"      { return BOOL;}

"for"       { return FOR;}
"while"     { return WHILE;}
"do"        { return DO;}
"return"    { return RETURN;}
"break"     { return BREAK;}
"continue"  { return CONTINUE;}
"switch"    { return SWITCH;}
"case"      { return CASE;}
"default"   { return DEFAULT;}
"void"      { return VOID;}
"printf"    { return PRINTF;}
"scanf"     { return SCANF;}
"include"   { return INCLUDE;}
"stdio.h"   { return STDIO_H;}
"stdlib.h"  { return STDLIB_H;}
"main"      { return MAIN;}


"=="        { printf("DEQ\n");}
"!="        { printf("NEQ\n");}
"<="        { printf("LE\n");}
">="        { printf("GE\n");}
"<"         { printf("LT\n");}
">"         { printf("GT\n");}
"="         { return EQUAL;}

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULT; }
"/"         { return DIV; }
"%"         { return MOD; }
"++"        { return INCREMENT;}
"--"        { return DECREMENT;}
"+="        { return ADD_EQUAL;}
"-="        { return SUB_EQUAL;}
"*="        { return MULT_EQUAL;}
"/="        { return DIV_EQUAL;}
"%="        { return MOD_EQUAL;}

"!"         { return NOT;}
"&"         { return AND;}
"|"         { return OR;}

";"         { return PONTO_VIRGULA;}
"("         { return OPEN_PAREN;}
")"         { return CLOSE_PAREN;}
"{"         { printf("Abre_Chaves\n");}
"}"         { printf("Fecha_Chaves\n");}

"'"         { return ASPASSIMPLES;}


[0-9]+"."[0-9]+ { return NUM;}
[0-9]+          { return NUM;}

[a-zA-Z][a-zA-Z0-9]*  { return ID; }

[ \t\n]      {}


.           { printf("DESCONHECIDO(%s)\n", yytext);}

%%


int yywrap(){
    return 1;
}