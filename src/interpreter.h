#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "tipos.h"

/* Valor em tempo de execução */
typedef struct Valor {
    Tipo tipo;
    union {
        int   i;
        float f;
        char  c;
    } dado;
} Valor;

/* Variável em tempo de execução */
typedef struct VarRT {
    char    nome[64];
    Valor   valor;
    struct VarRT *prox;
} VarRT;

/* Escopo em tempo de execução */
typedef struct EscopoRT {
    VarRT           *vars;
    struct EscopoRT *anterior;
} EscopoRT;

/* Sinal de controle de fluxo — usado para propagar 'return' */
typedef struct {
    int   ativo;   /* 1 se um return foi executado */
    Valor valor;   /* valor retornado */
} ReturnSinal;

void interpretarPrograma(NoAST *raiz);

#endif