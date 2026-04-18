%{
#include "parserC.tab.h"
#include <stdio.h>
#include <stdlib.h>

int linha = 1;
int coluna = 1;

char linhaAtual[1024];
int posLinha = 0;

#define ADD_TEXTO() \
    if (posLinha + yyleng < 1023) { \
        strncpy(&linhaAtual[posLinha], yytext, yyleng); \
        posLinha += yyleng; \
        linhaAtual[posLinha] = '\0'; \
    }

%}

%%

\n {
    linhaAtual[posLinha] = '\0';
    linha++;
    coluna = 1;
    posLinha = 0;
}

[ \t]+ {
    ADD_TEXTO();
    coluna += yyleng;
}


"if"        { coluna += yyleng; return IF;}
"else"      { coluna += yyleng; return ELSE;}

"int"       { ADD_TEXTO(); coluna += yyleng; return INT;}
"float"     { coluna += yyleng; return FLOAT;}
"char"      { coluna += yyleng; return CHAR;}
"bool"      { coluna += yyleng; return BOOL;}

"for"       { coluna += yyleng; return FOR;}
"return"    { coluna += yyleng; return RETURN;}
"break"     { coluna += yyleng; return BREAK;}
"void"      { coluna += yyleng; return VOID;}
"printf"    { coluna += yyleng; return PRINTF;}
"scanf"     { coluna += yyleng; return SCANF;}


"=="        { printf("DEQ\n");}
"!="        { printf("NEQ\n");}
"<="        { printf("LE\n");}
">="        { printf("GE\n");}
"<"         { printf("LT\n");}
">"         { printf("GT\n");}
"="         { ADD_TEXTO(); coluna += yyleng; return EQUAL;}

"+"         { coluna += yyleng; return PLUS; }
"-"         { coluna += yyleng; return MINUS; }
"*"         { coluna += yyleng; return MULT; }
"/"         { coluna += yyleng; return DIV; }
"%"         { coluna += yyleng; return MOD; }
"++"        { coluna += yyleng; return INCREMENT;}
"--"        { coluna += yyleng; return DECREMENT;}
"+="        { coluna += yyleng; return ADD_EQUAL;}
"-="        { coluna += yyleng; return SUB_EQUAL;}
"*="        { coluna += yyleng; return MULT_EQUAL;}
"/="        { coluna += yyleng; return DIV_EQUAL;}
"%="        { coluna += yyleng; return MOD_EQUAL;}

"!"         { coluna += yyleng; return NOT;}
"&&"        { coluna += yyleng; return AND;}
"||"        { coluna += yyleng; return OR;}

";"         { ADD_TEXTO(); coluna += yyleng; return PONTO_VIRGULA;}
"("         { coluna += yyleng; return OPEN_PAREN;}
")"         { coluna += yyleng; return CLOSE_PAREN;}
"{"         { coluna += yyleng; return ABRE_CHAVES;}
"}"         { coluna += yyleng; return FECHA_CHAVES;}
","         { coluna += yyleng; return VIRGULA;}
"'"         { coluna += yyleng; return ASPASSIMPLES;}


([0-9]+\.[0-9]*|\.[0-9]+) {            
    if (posLinha + yyleng < 1023) {
        strncpy(&linhaAtual[posLinha], yytext, yyleng);
        posLinha += yyleng;
    }
    
    yylval.floatValue = atof(yytext);   
    coluna += yyleng;
    return FLOAT_NUM;
}

[0-9]+ {
    if (posLinha + yyleng < 1023) {
        strncpy(&linhaAtual[posLinha], yytext, yyleng);
        posLinha += yyleng;
    }

    yylval.intValue = atoi(yytext);
    coluna += yyleng;
    return INT_NUM;
}


[a-zA-Z][a-zA-Z0-9]* {
    ADD_TEXTO();
    yylval.id = strdup(yytext);
    coluna += yyleng;
    return ID;
}

\'[a-zA-Z]\' {
    if (posLinha + yyleng < 1023) {
        strncpy(&linhaAtual[posLinha], yytext, yyleng);
        posLinha += yyleng;
    }

    yylval.intValue = yytext[1];
    coluna += yyleng;
    return CHAR_NUM;
}



. {
    printf("ERROR LEXER\n");
    printf("Line: %d Column:% d | error: invalid character '%s'\n", linha, coluna, yytext);
    printf("    %s\n", linhaAtual);
    printf("    ");
    for (int i = 1; i < coluna; i++) printf(" ");
    printf("^\n");

    coluna++;
}
%%


int yywrap(){
    return 1;
}