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
"return"    { return RETURN;}
"break"     { return BREAK;}
"void"      { return VOID;}
"printf"    { return PRINTF;}
"scanf"     { return SCANF;}


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
"&&"        { return AND;}
"||"        { return OR;}

";"         { return PONTO_VIRGULA;}
"("         { return OPEN_PAREN;}
")"         { return CLOSE_PAREN;}
"{"         { return ABRE_CHAVES;}
"}"         { return FECHA_CHAVES;}
","         { return VIRGULA;}
"'"         { return ASPASSIMPLES;}


([0-9]+\.[0-9]*|\.[0-9]+) {             // Captura valores ponto flutuante
    yylval.floatValue = atof(yytext);   // floatValue tipo float
    return FLOAT_NUM;
}

[0-9]+ {
    yylval.intValue = atoi(yytext);
    return INT_NUM;
}


[a-zA-Z][a-zA-Z0-9]*  { return ID; }

[ \t\n]+      {}


.           { printf("DESCONHECIDO(%s)\n", yytext);}

%%


int yywrap(){
    return 1;
}