%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "tac.h"
#include "tabsym.h"
#include "semantic.h"
#include "interpreter.h"

int  yylex(void);
void yyerror(const char *s);
NoAST *root = NULL;
int   debug = 0;

extern int   linha;
extern int   coluna;
extern char  linhaAtual[1024];
extern char *yytext;
%}

%define parse.error simple

%code requires {
    #include "ast.h"
}

%union {
    int    intValue;
    float  floatValue;
    char  *id;
    NoAST *ast;
}

%token <intValue>   INT_NUM CHAR_NUM BOOL_VAL
%token <floatValue> FLOAT_NUM
%token <id>         ID STRING_LITERAL

%token IF ELSE WHILE FOR RETURN BREAK VOID
%token INT FLOAT CHAR BOOL
%token EQUAL PONTO_VIRGULA VIRGULA
%token OPEN_PAREN CLOSE_PAREN ABRE_CHAVES FECHA_CHAVES
%token PLUS MINUS MULT DIV MOD
%token INCREMENT DECREMENT
%token ADD_EQUAL SUB_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL
%token AND OR NOT
%token DEQ NEQ LE GE LT GT
%token PRINTF SCANF ASPASSIMPLES

%type <intValue> tipo
%type <ast> expressao lista elemento comando atribuicao
%type <ast> declaracao comandos bloco lista_ids
%type <ast> incr_expr args_printf

/* precedência — do menor para o maior */
%right EQUAL ADD_EQUAL SUB_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL
%left  OR
%left  AND
%left  DEQ NEQ
%left  LT GT LE GE
%left  PLUS MINUS
%left  MULT DIV MOD
%right NOT
%right INCREMENT DECREMENT   /* prefixo */

%%

programa:
    lista {
        root = $1;
        analisarSemantica(root);
        if (debug) imprimirTabela();
        if (errosSemanticos() > 0) {
            fprintf(stderr, "\n%d erro(s) semantico(s) encontrado(s).\n",
                    errosSemanticos());
        } else {
            if (debug) {
                printf("\nTAC do programa:\n");
                gerarTAC(root);
                printf("\n");
            }
            interpretarPrograma(root);
        }
    }
;

lista:
    lista elemento  { $$ = criarNoSeq($1, $2); }
  | elemento        { $$ = $1; }
;

elemento: comando ;

bloco:
    ABRE_CHAVES { entrarEscopo(); } comandos FECHA_CHAVES { sairEscopo(); $$ = $3; }
  | ABRE_CHAVES { entrarEscopo(); }          FECHA_CHAVES { sairEscopo(); $$ = NULL; }
;

comandos:
    comandos comando  { $$ = criarNoSeq($1, $2); }
  | comando           { $$ = $1; }
;

comando:
    expressao PONTO_VIRGULA          { $$ = $1; }
  | atribuicao                       { $$ = $1; }
  | declaracao                       { $$ = $1; }
  | bloco                            { $$ = $1; }
  | error PONTO_VIRGULA              { yyerrok; yyclearin; $$ = NULL; }

  | IF OPEN_PAREN expressao CLOSE_PAREN bloco
      { $$ = criarNoIf($3, $5, NULL); }

  | IF OPEN_PAREN expressao CLOSE_PAREN bloco ELSE bloco
      { $$ = criarNoIf($3, $5, $7); }

  | WHILE OPEN_PAREN expressao CLOSE_PAREN bloco
      { $$ = criarNoWhile($3, $5); }

  | FOR OPEN_PAREN for_init expressao PONTO_VIRGULA incr_expr CLOSE_PAREN bloco
      { $$ = criarNoFor($<ast>3, $4, $6, $8); }

  | PRINTF OPEN_PAREN STRING_LITERAL CLOSE_PAREN PONTO_VIRGULA
      { $$ = criarNoPrintfFmt(criarNoString($3), NULL); free($3); }

  | PRINTF OPEN_PAREN STRING_LITERAL VIRGULA args_printf CLOSE_PAREN PONTO_VIRGULA
      { $$ = criarNoPrintfFmt(criarNoString($3), $5); free($3); }

  | PRINTF OPEN_PAREN expressao CLOSE_PAREN PONTO_VIRGULA
      { $$ = criarNoPrintf($3); }
;

/* init do for pode ser declaração ou atribuição ou vazio */
for_init:
    declaracao       { $<ast>$ = $1; }
  | atribuicao       { $<ast>$ = $1; }
  | PONTO_VIRGULA    { $<ast>$ = NULL; }
;

/* incremento do for: expr composta ou simples */
incr_expr:
    ID ADD_EQUAL  expressao  { $$ = criarNoOp('a', criarNoId($1), $3); }
  | ID SUB_EQUAL  expressao  { $$ = criarNoOp('s', criarNoId($1), $3); }
  | ID MULT_EQUAL expressao  { $$ = criarNoOp('m', criarNoId($1), $3); }
  | ID DIV_EQUAL  expressao  { $$ = criarNoOp('v', criarNoId($1), $3); }
  | ID MOD_EQUAL  expressao  { $$ = criarNoOp('r', criarNoId($1), $3); }
  | ID INCREMENT             { $$ = criarNoOp('I', criarNoId($1), NULL); }
  | ID DECREMENT             { $$ = criarNoOp('D', criarNoId($1), NULL); }
  | INCREMENT ID             { $$ = criarNoOp('I', criarNoId($2), NULL); }
  | DECREMENT ID             { $$ = criarNoOp('D', criarNoId($2), NULL); }




  | expressao                { $$ = $1; }
  | /* vazio */              { $$ = NULL; }
;

args_printf:
    expressao
        { $$ = criarListaArgs($1, NULL); }
  | args_printf VIRGULA expressao
        { $$ = criarListaArgs($3, $1); }
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
        /* propaga o tipo correto por toda a lista de declarações */
        NoAST *aux = $$;
        while (aux != NULL) {
            if (aux->operador == ';') {
                /* nó sequência: o filho esquerdo é o nó 'd' desta variável */
                NoAST *decl = aux->esquerda;
                while (decl && decl->operador == ';') decl = decl->esquerda;
                if (decl && decl->operador == 'd') decl->tipo = $1;
                aux = aux->direita;
            } else {
                /* nó 'd' terminal */
                aux->tipo = $1;
                break;
            }
        }
    }
;

lista_ids:
    ID
      { $$ = criarNoDecl(T_INT, $1, NULL); }
  | ID EQUAL expressao
      { $$ = criarNoDecl(T_INT, $1, $3); }
  | lista_ids VIRGULA ID
      { $$ = criarNoSeq($1, criarNoDecl(T_INT, $3, NULL)); }
  | lista_ids VIRGULA ID EQUAL expressao
      { $$ = criarNoSeq($1, criarNoDecl(T_INT, $3, $5)); }
;

atribuicao:
    ID EQUAL      expressao PONTO_VIRGULA { $$ = criarNoAtrib($1, $3); }
  | ID ADD_EQUAL  expressao PONTO_VIRGULA { $$ = criarNoOp('a', criarNoId($1), $3); }
  | ID SUB_EQUAL  expressao PONTO_VIRGULA { $$ = criarNoOp('s', criarNoId($1), $3); }
  | ID MULT_EQUAL expressao PONTO_VIRGULA { $$ = criarNoOp('m', criarNoId($1), $3); }
  | ID DIV_EQUAL  expressao PONTO_VIRGULA { $$ = criarNoOp('v', criarNoId($1), $3); }
  | ID MOD_EQUAL  expressao PONTO_VIRGULA { $$ = criarNoOp('r', criarNoId($1), $3); }
  | INCREMENT ID PONTO_VIRGULA            { $$ = criarNoOp('I', criarNoId($2), NULL); }
  | DECREMENT ID PONTO_VIRGULA            { $$ = criarNoOp('D', criarNoId($2), NULL); }
  | ID INCREMENT PONTO_VIRGULA            { $$ = criarNoOp('I', criarNoId($1), NULL); }
  | ID DECREMENT PONTO_VIRGULA            { $$ = criarNoOp('D', criarNoId($1), NULL); }
  | ID EQUAL error PONTO_VIRGULA          { yyerrok; $$ = NULL; }
;

expressao:
    expressao PLUS  expressao  { $$ = criarNoOp('+', $1, $3); }
  | expressao MINUS expressao  { $$ = criarNoOp('-', $1, $3); }
  | expressao MULT  expressao  { $$ = criarNoOp('*', $1, $3); }
  | expressao DIV   expressao  { $$ = criarNoOp('/', $1, $3); }
  | expressao MOD   expressao  { $$ = criarNoOp('%', $1, $3); }
  | expressao DEQ   expressao  { $$ = criarNoOp('e', $1, $3); }
  | expressao NEQ   expressao  { $$ = criarNoOp('!', $1, $3); }
  | expressao LT    expressao  { $$ = criarNoOp('<', $1, $3); }
  | expressao GT    expressao  { $$ = criarNoOp('>', $1, $3); }
  | expressao LE    expressao  { $$ = criarNoOp('L', $1, $3); }
  | expressao GE    expressao  { $$ = criarNoOp('G', $1, $3); }
  | expressao AND   expressao  { $$ = criarNoAnd($1, $3); }
  | expressao OR    expressao  { $$ = criarNoOr($1, $3); }
  | NOT expressao              { $$ = criarNoNot($2); }


  | OPEN_PAREN expressao CLOSE_PAREN { $$ = $2; }
  | INT_NUM    { $$ = criarNoInt($1); }
  | FLOAT_NUM  { $$ = criarNoFloat($1); }
  | CHAR_NUM   { $$ = criarNoChar((char)$1); }
  | BOOL_VAL   { $$ = criarNoBool($1); }
  | ID         { $$ = criarNoId($1); }
;

%%

int main(int argc, char *argv[]) {
    extern FILE *yyin;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        } else {
            FILE *f = fopen(argv[i], "r");
            if (f == NULL) {
                fprintf(stderr, "Erro: nao foi possivel abrir '%s'\n", argv[i]);
                return 1;
            }
            yyin = f;
        }
    }

    entrarEscopo();
    int ret = yyparse();
    if (yyin != stdin) fclose(yyin);
    return ret;
}

void yyerror(const char *s) {
    (void)s;
    printf("ERROR PARSER\n");
    printf("Line: %d Column: %d | error: unexpected token: '%s'\n",
           linha, coluna, yytext);
    printf("    %s\n", linhaAtual);
    printf("    ");
    for (int i = 1; i < coluna; i++) printf(" ");
    printf("^\n\n");
}