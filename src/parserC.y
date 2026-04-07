%{
#include <stdio.h>


int yylex(void);
void yyerror(const char *s);



%}

%token IF ELSE INT ID FLOAT CHAR FOR
%token NUM
%token EQUAL ASPASSIMPLES
%token OPEN_PAREN CLOSE_PAREN
%token PONTO_VIRGULA
%token PLUS MINUS MULT DIV
%token INCREMENT DECREMENT ADD_EQUAL SUB_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL
%token AND OR NOT
%token MOD WHILE DO
%token VOID MAIN
%token PRINTF SCANF INCLUDE STDIO_H STDLIB_H MATH_H
%token SWITCH CASE DEFAULT
%token RETURN BREAK CONTINUE
%token BOOL


%%
programa:
    lista
;

lista:
    lista elemento
    | elemento
;

elemento:
    declaracao
    | atribuicao
    | comando
;


declaracao:
    INT ID PONTO_VIRGULA
    | FLOAT ID PONTO_VIRGULA
    | CHAR ID PONTO_VIRGULA
    ;

    
atribuicao:
    INT ID EQUAL NUM PONTO_VIRGULA
    | FLOAT ID EQUAL NUM PONTO_VIRGULA
    | CHAR ID EQUAL ASPASSIMPLES ID ASPASSIMPLES PONTO_VIRGULA
    ;


comando:
    IF OPEN_PAREN expressao CLOSE_PAREN comando
    | ID EQUAL expressao PONTO_VIRGULA
    ;

expressao:
    expressao PLUS expressao
    | NUM
    | ID
    ;

%% 

int main(){
    return yyparse();
}

void yyerror(const char *s){
    printf("Erro sintatico: %s\n", s);
}