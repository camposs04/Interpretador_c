#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabsym.h"
#include "ast.h"
#include "semantic.h"

static int numErros = 0;

void erroSemantico(int codigoErro, int linha) {
    numErros++;
    if (codigoErro == ERR_DIVISAO_POR_ZERO) {
        printf("Erro Semantico: divisao por zero na linha %d.\n", linha);
    }
}

int errosSemanticos(void) { return numErros; }

static const char *tipoStr(Tipo t) {
    switch (t) {
        case T_INT:   return "int";
        case T_FLOAT: return "float";
        case T_CHAR:  return "char";
        case T_BOOL:  return "bool";
        case T_FUNC:  return "func";
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

        case 'I': case 'D':
            analisarSemantica(raiz->esquerda);
            return;

        case ';':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            return;

        /* if/else */
        case 'f': {
            analisarSemantica(raiz->esquerda);
            NoAST *corpo = raiz->direita;
            entrarEscopo();
            analisarSemantica(corpo->esquerda);
            sairEscopo();
            if (corpo->direita) {
                entrarEscopo();
                analisarSemantica(corpo->direita);
                sairEscopo();
            }
            return;
        }

        case 'W':
            analisarSemantica(raiz->esquerda);
            entrarEscopo();
            analisarSemantica(raiz->direita);
            sairEscopo();
            return;

        case 'F': {
            NoAST *meta  = raiz->esquerda;
            NoAST *resto = raiz->direita;
            entrarEscopo();
            analisarSemantica(meta->esquerda);
            analisarSemantica(resto->esquerda);
            analisarSemantica(meta->direita);
            entrarEscopo();
            analisarSemantica(resto->direita);
            sairEscopo();
            sairEscopo();
            return;
        }

        case 'P':
            analisarSemantica(raiz->esquerda);
            return;

        case 'R': {
            NoAST *cur = raiz->direita;
            while (cur != NULL) {
                if (cur->operador == 'L') {
                    analisarSemantica(cur->esquerda);
                    cur = cur->direita;
                } else {
                    analisarSemantica(cur);
                    break;
                }
            }
            return;
        }

        case 'A': case 'O':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            raiz->tipo = T_BOOL;
            return;

        case 'N':
            analisarSemantica(raiz->esquerda);
            raiz->tipo = T_BOOL;
            return;

        /* ── definição de função ── */
        case 'Z': {
            /* Registra a função no escopo global antes de analisar o corpo,
               permitindo recursão. */
            if (searchSymbolEscopoAtual(raiz->nome) != NULL) {
                printf("Erro Semantico: funcao '%s' ja declarada.\n", raiz->nome);
                numErros++;
            } else {
                insertFuncao(raiz->nome, raiz->tipo, raiz->params, raiz->esquerda);
            }

            /* Abre escopo para parâmetros + corpo */
            entrarEscopo();
            for (Param *p = raiz->params; p != NULL; p = p->prox)
                insertSymbol(p->nome, tipoStr(p->tipo));
            analisarSemantica(raiz->esquerda);
            sairEscopo();
            return;
        }

        /* ── chamada de função ── */
        case 'C': {
            Symb *s = searchSymbol(raiz->nome);
            if (!s || !s->isFuncao) {
                printf("Erro Semantico: funcao '%s' nao declarada.\n", raiz->nome);
                numErros++;
                return;
            }
            /* Propaga tipo de retorno */
            raiz->tipo = s->retorno;

            /* Verifica número de argumentos */
            int nparams = 0;
            for (Param *p = s->params; p; p = p->prox) nparams++;
            int nargs = 0;
            for (NoAST *cur = raiz->esquerda; cur != NULL; ) {
                nargs++;
                if (cur->operador == 'L') cur = cur->direita;
                else break;
            }
            if (nargs != nparams) {
                printf("Erro Semantico: funcao '%s' espera %d argumento(s), recebeu %d.\n",
                       raiz->nome, nparams, nargs);
                numErros++;
            }

            /* Analisa cada argumento */
            NoAST *cur = raiz->esquerda;
            while (cur != NULL) {
                if (cur->operador == 'L') {
                    analisarSemantica(cur->esquerda);
                    cur = cur->direita;
                } else {
                    analisarSemantica(cur);
                    break;
                }
            }
            return;
        }

        /* ── return ── */
        case 'K':
            analisarSemantica(raiz->esquerda);
            return;

        default:
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            if (raiz->esquerda && raiz->direita) {
                Tipo te = raiz->esquerda->tipo;
                Tipo td = raiz->direita->tipo;
                raiz->tipo = (te == T_FLOAT || td == T_FLOAT) ? T_FLOAT : T_INT;
            }
            return;
    }
}