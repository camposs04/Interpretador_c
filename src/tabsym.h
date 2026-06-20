#ifndef TABSYM_H
#define TABSYM_H

#include "tipos.h"   /* Param, NoAST */

#define TABLE_SIZE 211

typedef struct Symb {
    char        *name;
    char        *type;       /* tipo como string: "int", "float", ... ou "func" */
    Tipo         retorno;    /* tipo de retorno, usado se isFuncao == 1 */
    int          isFuncao;
    Param       *params;     /* lista de parâmetros formais (apenas funções) */
    struct noAST *corpo;     /* AST do corpo (apenas funções) */
    int          isVetor;    /* 1 se este símbolo é um vetor */
    int          tamanho;    /* número de elementos (apenas vetores) */
    struct Symb  *next;
} Symb;

typedef struct Escopo {
    Symb          *tabela[TABLE_SIZE];
    struct Escopo *anterior;
} Escopo;

void  entrarEscopo(void);
void  sairEscopo(void);
void  insertSymbol(const char *name, const char *type);
void  insertVetor(const char *name, const char *type, int tamanho);
void  insertFuncao(const char *name, Tipo retorno, Param *params, struct noAST *corpo);
Symb *searchSymbol(const char *name);
Symb *searchSymbolEscopoAtual(const char *name);
void  imprimirTabela(void);

#endif