#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabsym.h"
#include "ast.h"
#include "semantic.h"

static int numErros = 0;

int errosSemanticos(void) { return numErros; }

static const char *tipoStr(Tipo t) {
    switch (t) {
        case T_INT:   return "int";
        case T_FLOAT: return "float";
        case T_CHAR:  return "char";
        case T_BOOL:  return "bool";
        default:      return "void";
    }
}

static void declaration(NoAST *raiz) {
    const char *nome     = raiz->esquerda->nome;
    const char *tipo_str = tipoStr(raiz->tipo);

    if (searchSymbolEscopoAtual(nome) != NULL) {
        printf("Erro Semantico: variavel '%s' ja declarada neste escopo.\n", nome);
        numErros++;
    } else {
        insertSymbol(nome, tipo_str);
    }

    if (raiz->direita) analisarSemantica(raiz->direita);
}

static void identifier(NoAST *raiz) {
    Symb *s = searchSymbol(raiz->nome);
    if (!s) {
        printf("Erro Semantico: variavel '%s' nao declarada.\n", raiz->nome);
        numErros++;
        return;
    }
    if      (strcmp(s->type,"int")   == 0) raiz->tipo = T_INT;
    else if (strcmp(s->type,"float") == 0) raiz->tipo = T_FLOAT;
    else if (strcmp(s->type,"char")  == 0) raiz->tipo = T_CHAR;
    else if (strcmp(s->type,"bool")  == 0) raiz->tipo = T_BOOL;
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

        /* atribuição e operadores compostos */
        case '=':
        case 'a': case 's': case 'm': case 'v': case 'r':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

        /* ++ / -- têm apenas filho esquerdo */
        case 'I': case 'D':
            analisarSemantica(raiz->esquerda);
            return;

        case ';':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

        /* if/else */
        case 'f': {
            analisarSemantica(raiz->esquerda);   /* condição */
            NoAST *corpo = raiz->direita;
            entrarEscopo();
            analisarSemantica(corpo->esquerda);  /* bloco then */
            sairEscopo();
            if (corpo->direita) {
                entrarEscopo();
                analisarSemantica(corpo->direita); /* bloco else */
                sairEscopo();
            }
            return;
        }

        /* while */
        case 'W':
            analisarSemantica(raiz->esquerda);   /* condição */
            entrarEscopo();
            analisarSemantica(raiz->direita);    /* corpo */
            sairEscopo();
            return;

        /* for: esquerda = meta(init,incr), direita = seq(cond,corpo) */
        case 'F': {
            NoAST *meta  = raiz->esquerda;
            NoAST *resto = raiz->direita;
            entrarEscopo();                      /* escopo do for */
            analisarSemantica(meta->esquerda);   /* init */
            analisarSemantica(resto->esquerda);  /* cond */
            analisarSemantica(meta->direita);    /* incr */
            entrarEscopo();
            analisarSemantica(resto->direita);   /* corpo */
            sairEscopo();
            sairEscopo();
            return;
        }

        /* printf */
        case 'P':
            analisarSemantica(raiz->esquerda);
            return;

        /* operadores lógicos */
        case 'A': case 'O':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            raiz->tipo = T_BOOL;
            return;

        case 'N':
            analisarSemantica(raiz->esquerda);
            raiz->tipo = T_BOOL;
            return;

        default:
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);

            /* propaga tipo após resolução dos filhos */
            if (raiz->esquerda && raiz->direita) {
                Tipo te = raiz->esquerda->tipo;
                Tipo td = raiz->direita->tipo;
                raiz->tipo = (te == T_FLOAT || td == T_FLOAT) ? T_FLOAT : T_INT;
            }
            return;
    }
}