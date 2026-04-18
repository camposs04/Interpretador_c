%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include <string.h>

int yylex(void);
void yyerror(const char *s);
NoAST *root = NULL;

extern int linha;
extern int coluna;
extern char linhaAtual[1024];
extern char *yytext;
%}

%define parse.error simple

%code requires {
    #include "ast.h"
}
%union{
    int intValue;       
    float floatValue;
    char *id;
    NoAST *ast;
}

%token <intValue> INT_NUM
%token <floatValue> FLOAT_NUM
%token <intValue> CHAR_NUM
%type <intValue> tipo

%token <id> ID

%token IF ELSE INT FLOAT CHAR FOR
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
%token DEQ NEQ LE GE LT GT

%left DEQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MULT DIV

%type <ast> expressao    
%type <ast> lista elemento
%type <ast> comando
%type <ast> atribuicao
%type <ast> declaracao
%type <ast> comandos
%type <ast> bloco
%type <ast> lista_ids

%%

programa:
    lista {
        root = $1;
        printf("\nAST do programa:\n");
        imprimirAST(root);
        printf("\n");
    }
;

lista:
    lista elemento { $$ = criarNoSeq($1, $2); }
    | elemento     { $$ = $1; }
;

elemento:
    comando
;

bloco:
    ABRE_CHAVES comandos FECHA_CHAVES { $$ = $2; }
;


comandos:
    comandos comando { $$ = criarNoSeq($1, $2); }
    | comando { $$ = $1; }
;

comando:
    expressao PONTO_VIRGULA { $$ = $1; }
    | atribuicao           { $$ = $1; }
    | declaracao           { $$ = $1; }
    | bloco                { $$ = $1; }
    | error PONTO_VIRGULA  { yyerrok; yyclearin; $$ = NULL; }
    | IF OPEN_PAREN expressao CLOSE_PAREN bloco {
        $$ = criarNoIf($3, $5, NULL);
    }
    | IF OPEN_PAREN expressao CLOSE_PAREN bloco ELSE bloco {
        $$ = criarNoIf($3, $5, $7);
    }
;

tipo:
    INT   { $$ = T_INT; }
    | FLOAT { $$ = T_FLOAT; }
    | CHAR  { $$ = T_CHAR; }
    | BOOL  { $$ = T_BOOL; }
;

declaracao:
    tipo lista_ids PONTO_VIRGULA {
        $$ = $2; 
        NoAST *aux = $$;
        while (aux != NULL) {
            if (aux->operador == ';') { 
                if (aux->esquerda) aux->esquerda->tipo = $1;
                aux = aux->direita;
            } else { 
                aux->tipo = $1;
                break;
            }
        }
    }
;

lista_ids:
    ID { 
        $$ = criarNoDecl(T_INT, $1, NULL); 
    }
    | ID EQUAL expressao {
        $$ = criarNoDecl(T_INT, $1, $3);
    }
    | lista_ids VIRGULA ID {
        NoAST *novo = criarNoDecl(T_INT, $3, NULL);
        $$ = criarNoSeq($1, novo);
    }
    | lista_ids VIRGULA ID EQUAL expressao {
        NoAST *novo = criarNoDecl(T_INT, $3, $5);
        $$ = criarNoSeq($1, novo);
    }
;


atribuicao:
    ID EQUAL expressao PONTO_VIRGULA { 
        $$ = criarNoAtrib($1, $3); 
    }
    | ID EQUAL error PONTO_VIRGULA {
        yyerrok;
        $$ = NULL; 
    }
;

expressao:
    expressao PLUS expressao    { $$ = criarNoOp('+', $1, $3); }
    | expressao MINUS expressao { $$ = criarNoOp('-', $1, $3); }
    | expressao MULT expressao  { $$ = criarNoOp('*', $1, $3); }
    | expressao DIV expressao   { $$ = criarNoOp('/', $1, $3); }
    | expressao DEQ expressao   { $$ = criarNoOp('e', $1, $3); } 
    | expressao NEQ expressao   { $$ = criarNoOp('!', $1, $3); } 
    | expressao LT expressao    { $$ = criarNoOp('<', $1, $3); } 
    | expressao GT expressao    { $$ = criarNoOp('>', $1, $3); }
    | expressao LE expressao    { $$ = criarNoOp('L', $1, $3); } 
    | expressao GE expressao    { $$ = criarNoOp('G', $1, $3); } 
    | OPEN_PAREN expressao CLOSE_PAREN { $$ = $2; }
    | INT_NUM { $$ = criarNoInt($1); }
    | FLOAT_NUM { $$ = criarNoFloat($1); }
    | CHAR_NUM { $$ = criarNoChar($1); }
    | ID { $$ = criarNoId($1); }
;

%%

int main(){
    return yyparse();
}

void yyerror(const char *s){
    (void)s; 

    printf("ERROR PARSER\n");
    printf("Line: %d Column: %d | error: unexpected token: '%s'\n", linha, coluna, yytext);

    printf("    %s\n", linhaAtual);
    printf("    ");
    for (int i = 1; i < coluna; i++) printf(" ");
    printf("^\n\n");
}