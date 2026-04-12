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
    expressao PONTO_VIRGULA { printf("%d\n", $1); }
    | IF OPEN_PAREN expressao CLOSE_PAREN comando
    | ID EQUAL expressao PONTO_VIRGULA
    ;

expressao:
      expressao PLUS expressao          { $$ = $1 + $3; }
    | expressao MINUS expressao         { $$ = $1 - $3; }
    | expressao MULT expressao          { $$ = $1 * $3; }
    | expressao DIV expressao           { $$ = $1 / $3; }
    | OPEN_PAREN expressao CLOSE_PAREN  { $$ = $2; }
    | NUM                               { $$ = $1; }
    ;

%% 

int main(){
    return yyparse();
}

void yyerror(const char *s){
    printf("Erro sintatico: %s\n", s);
}