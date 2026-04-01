%{
#include <stdio.h>


int yylex(void);
void yyerror(const char *s);



%}

%token IF ELSE INT ID FLOAT CHAR FOR
%token NUM
%token EQUAL
%token OPEN_PAREN CLOSE_PAREN
%token PONTO_VIRGULA
%token PLUS MINUS MULT DIV
%%



atribuicao:
    INT ID EQUAL NUM PONTO_VIRGULA
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