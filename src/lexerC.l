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

"for"       { return FOR;}

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

";"         { return PONTO_VIRGULA;}
"("         { return OPEN_PAREN;}
")"         { return CLOSE_PAREN;}
"{"         { printf("Abre_Chaves\n");}
"}"         { printf("Fecha_Chaves\n");}

[0-9]+"."[0-9]+ { return NUM;}
[0-9]+          { return NUM;}

[a-zA-Z][a-zA-Z0-9]*  { return ID; }

[ \t\n]      {}


.           { printf("DESCONHECIDO(%s)\n", yytext);}

%%


int yywrap(){
    return 1;
}