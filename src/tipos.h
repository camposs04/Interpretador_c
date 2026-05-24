#ifndef TIPOS_H
#define TIPOS_H

typedef enum {
    T_INT,
    T_FLOAT,
    T_CHAR,
    T_BOOL,
    T_VOID,
    T_FUNC
} Tipo;

/* Parâmetro formal de função */
typedef struct Param {
    Tipo        tipo;
    char        nome[64];
    struct Param *prox;
} Param;

typedef struct noAST {
    char operador;
    Tipo tipo;

    union {
        int   i;
        float f;
        char  c;
    } valor;

    char nome[256];

    struct noAST *esquerda;
    struct noAST *direita;

    /* usado apenas em nós de definição de função ('Z') */
    Param *params;
} NoAST;

#endif