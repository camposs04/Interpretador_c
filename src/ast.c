#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NoAST *criarNoInt(int valor) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = 'n';
    novo->tipo = T_INT;
    novo->valor.i = valor;
    novo->esquerda = novo->direita = NULL;
    return novo;
}

NoAST *criarNoFloat(float valor) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = 'n';
    novo->tipo = T_FLOAT;
    novo->valor.f = valor;
    novo->esquerda = novo->direita = NULL;
    return novo;
}

NoAST *criarNoId(char *nome) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = 'i';
    strcpy(novo->nome, nome);
    novo->esquerda = novo->direita = NULL;
    return novo;
}

NoAST *criarNoOp(char operador, NoAST *esq, NoAST *dir) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = operador;

    if (esq->tipo == T_FLOAT || dir->tipo == T_FLOAT)
        novo->tipo = T_FLOAT;
    else
        novo->tipo = T_INT;

    novo->valor.i = 0;
    novo->nome[0] = '\0';
    novo->esquerda = esq;
    novo->direita = dir;
    return novo;
}

void imprimirAST(NoAST *raiz) {
    if (raiz == NULL) return;

    switch (raiz->operador) {

        case 'n':
            switch (raiz->tipo) {
                case T_INT:
                    printf("%d", raiz->valor.i);
                    break;
                case T_FLOAT:
                    printf("%f", raiz->valor.f);
                    break;
                case T_CHAR:
                    printf("'%c'", raiz->valor.c);
                    break;
                case T_BOOL:
                    printf("%s", raiz->valor.i ? "true" : "false");
                    break;
            }
            break;

        case 'i':
            printf("%s", raiz->nome);
            break;

        case 'd':
            switch (raiz->tipo) {
                case T_INT:   printf("int "); break;
                case T_FLOAT: printf("float "); break;
                case T_CHAR:  printf("char "); break;
                case T_BOOL:  printf("bool "); break;
            }

            imprimirAST(raiz->esquerda);

            if (raiz->direita != NULL) {
                printf(" = ");
                imprimirAST(raiz->direita);
            }
            break;

        case '=':
            imprimirAST(raiz->esquerda);
            printf(" = ");
            imprimirAST(raiz->direita);
            break;

       
        case ';':
            imprimirAST(raiz->esquerda);
            printf(";\n");
            imprimirAST(raiz->direita);
            break;

        default:
            printf("(");
            imprimirAST(raiz->esquerda);
            printf(" %c ", raiz->operador);
            imprimirAST(raiz->direita);
            printf(")");
            break;
    }
}
NoAST *criarNoSeq(NoAST *esq, NoAST *dir) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = ';';

    novo->tipo = T_INT;        
    novo->valor.i = 0;         

    novo->nome[0] = '\0';
    novo->esquerda = esq;
    novo->direita = dir;

    return novo;
}

NoAST *criarNoAtrib(char *nome, NoAST *valor) {
    NoAST *id = criarNoId(nome);
    return criarNoOp('=', id, valor);
}

NoAST *criarNoChar(char valor) {
    NoAST *novo = malloc(sizeof(NoAST));
    novo->operador = 'n';
    novo->tipo = T_CHAR;
    novo->valor.c = valor;
    novo->esquerda = novo->direita = NULL;
    return novo;
}

NoAST *criarNoDecl(Tipo tipo, char *nome, NoAST *valor) {
    NoAST *novo = malloc(sizeof(NoAST));

    novo->operador = 'd';
    novo->tipo = tipo;  

    NoAST *id = criarNoId(nome);

    novo->valor.i = 0;
    novo->nome[0] = '\0';

    novo->esquerda = id;
    novo->direita = valor;

    return novo;
}