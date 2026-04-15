%{
#include <stdio.h>

int yylex(void);
void yyerror(const char *s);
%}

%union{
    int intValue;
}

%token <intValue> NUM

%token IF ELSE INT ID FLOAT CHAR FOR WHILE
%token EQUAL ASPASSIMPLES
%token OPEN_PAREN CLOSE_PAREN ABRE_CHAVES FECHA_CHAVES
%token PONTO_VIRGULA VIRGULA
%token PLUS MINUS MULT DIV
%token INCREMENT DECREMENT ADD_EQUAL SUB_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL
%token AND OR NOT
%token MOD
%token VOID BREAK
%token PRINTF SCANF
%token RETURN
%token BOOL

%left PLUS MINUS
%left MULT DIV

%type <intValue> expressao

%%

programa:
    comandos
;

comandos:
    comandos comando
    | comando
;

comando:
    declaracao
    | atribuicao
    | desvio
    | expressao PONTO_VIRGULA   { printf("Resultado: %d\n", $1); }
    | bloco
    | PRINTF OPEN_PAREN ID CLOSE_PAREN PONTO_VIRGULA
;


bloco:
    ABRE_CHAVES comandos FECHA_CHAVES  
;

lista_ids:
    ID
    | lista_ids VIRGULA ID
;

declaracao:
    INT lista_ids PONTO_VIRGULA
    | FLOAT lista_ids PONTO_VIRGULA
    | CHAR lista_ids PONTO_VIRGULA
;

atribuicao:
    ID EQUAL expressao PONTO_VIRGULA
;

desvio:
    IF OPEN_PAREN expressao CLOSE_PAREN comando
;

expressao:
    expressao PLUS expressao            { $$ = $1 + $3; }
    | expressao MINUS expressao         { $$ = $1 - $3; }
    | expressao MULT expressao          { $$ = $1 * $3; }
    | expressao DIV expressao           { $$ = $1 / $3; }
    | OPEN_PAREN expressao CLOSE_PAREN  { $$ = $2; }
    | NUM                               { $$ = $1; }
    | ID                                { $$ = 0; } 
;

%% 

int main(){
    return yyparse();
}

void yyerror(const char *s){
    printf("Erro sintatico: %s\n", s);
}