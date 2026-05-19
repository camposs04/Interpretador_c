#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabsym.h"
#include "ast.h"

char* mapearTipoParaString(Tipo tipo) {
    switch (tipo) {
        case T_INT:   return "int";
        case T_FLOAT: return "float";
        case T_CHAR:  return "char";
        case T_BOOL:  return "bool";
        default:      return "unknown";
    }
}

NoAST *declaration(NoAST *raiz){
    char *namen = raiz->esquerda->nome;
    char *type_converted = mapearTipoParaString(raiz->tipo);

    if(searchSymbol(namen) != NULL){
        printf("Erro Semantico: Variavel '%s' ja declarada.\n", namen);

    }else{
        insertSymbol(namen, type_converted);
    }

    if(raiz->direita != NULL){
        analisarSemantica(raiz->direita);
    }
}

int *identifier(NoAST *raiz){
    Symb *simbolo = searchSymbol(raiz->nome);
    
    if(simbolo == NULL){
        printf("Erro Semantico: Variavel '%s' nao declarada.\n", raiz->nome);
    }else{

        if (strcmp(simbolo->type, "int") == 0) {
            raiz->tipo = T_INT;
        } else if (strcmp(simbolo->type, "float") == 0) {
            raiz->tipo = T_FLOAT;
        } else if (strcmp(simbolo->type, "char") == 0) {
            raiz->tipo = T_CHAR;
        } else if (strcmp(simbolo->type, "bool") == 0) {
            raiz->tipo = T_BOOL;
        }
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
            break;

        case '=':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

    }

    analisarSemantica(raiz->esquerda);
    analisarSemantica(raiz->direita);
}