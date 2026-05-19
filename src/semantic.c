#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabsym.h"
#include "ast.h"
#include "semantic.h"

char* mapearTipoParaString(Tipo tipo) {
    switch (tipo) {
        case T_INT:   return "int";
        case T_FLOAT: return "float";
        case T_CHAR:  return "char";
        case T_BOOL:  return "bool";
        default:      return "unknown";
    }
}

static void declaration(NoAST *raiz) {
    char *nome = raiz->esquerda->nome;
    char *tipo_str = mapearTipoParaString(raiz->tipo);

    if (searchSymbolEscopoAtual(nome) != NULL) {
        printf("Erro Semantico: Variavel '%s' ja declarada neste escopo.\n", nome);
    } else {
        insertSymbol(nome, tipo_str);
    }

    if (raiz->direita != NULL) {
        analisarSemantica(raiz->direita);
    }
}

static void identifier(NoAST *raiz) {
    Symb *simbolo = searchSymbol(raiz->nome);

    if (simbolo == NULL) {
        printf("Erro Semantico: Variavel '%s' nao declarada.\n", raiz->nome);
    } else {
        if      (strcmp(simbolo->type, "int")   == 0) raiz->tipo = T_INT;
        else if (strcmp(simbolo->type, "float") == 0) raiz->tipo = T_FLOAT;
        else if (strcmp(simbolo->type, "char")  == 0) raiz->tipo = T_CHAR;
        else if (strcmp(simbolo->type, "bool")  == 0) raiz->tipo = T_BOOL;
    }
}

void analisarSemantica(NoAST *raiz) {
    if (raiz == NULL) return;

    switch (raiz->operador) {

        case 'd':
            declaration(raiz);
            return;

        case 'i':
            identifier(raiz);
            return;

        case '=':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

        case ';':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

        case 'f': {
            /* analisa a condição no escopo atual */
            analisarSemantica(raiz->esquerda);

            NoAST *corpo = raiz->direita;

            /* bloco then */
            entrarEscopo();
            analisarSemantica(corpo->esquerda);
            sairEscopo();

            /* bloco else (opcional) */
            if (corpo->direita != NULL) {
                entrarEscopo();
                analisarSemantica(corpo->direita);
                sairEscopo();
            }
            return;
        }

        default:
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;
    }
}