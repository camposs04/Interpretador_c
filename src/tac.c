#include "tac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int contTemp  = 0;
static int contLabel = 0;

static char *novoTemp(void) {
    char *buf = malloc(16);
    snprintf(buf, 16, "t%d", ++contTemp);
    return buf;
}

static int proximoLabel(void) { return ++contLabel; }

static void novoLabel(char *buf, int n) { snprintf(buf, 16, "L%d", n); }

/* Libera apenas se não for NULL — evita double-free */
static void liberarTemp(char *t) { if (t) free(t); }

char *gerarTAC(NoAST *raiz) {
    if (raiz == NULL) return NULL;

    switch (raiz->operador) {

        case 'n': {
            char *t = novoTemp();
            switch (raiz->tipo) {
                case T_INT:   printf("%s = %d\n",  t, raiz->valor.i); break;
                case T_FLOAT: printf("%s = %f\n",  t, raiz->valor.f); break;
                case T_CHAR:  printf("%s = '%c'\n",t, raiz->valor.c); break;
                case T_BOOL:  printf("%s = %s\n",  t, raiz->valor.i ? "true":"false"); break;
                default: break;
            }
            return t;
        }

        case 'i': {
            /* retorna cópia do nome — sempre heap, sempre liberável */
            char *t = malloc(32);
            strncpy(t, raiz->nome, 31);
            t[31] = '\0';
            return t;
        }

        case 'd': {
            const char *tipo_str;
            switch (raiz->tipo) {
                case T_INT:   tipo_str="int";   break;
                case T_FLOAT: tipo_str="float"; break;
                case T_CHAR:  tipo_str="char";  break;
                case T_BOOL:  tipo_str="bool";  break;
                default:      tipo_str="?";     break;
            }
            printf("decl %s %s\n", tipo_str, raiz->esquerda->nome);
            if (raiz->direita) {
                char *val = gerarTAC(raiz->direita);
                printf("%s = %s\n", raiz->esquerda->nome, val);
                liberarTemp(val);
            }
            return NULL;
        }

        case '=': {
            char *val = gerarTAC(raiz->direita);
            printf("%s = %s\n", raiz->esquerda->nome, val);
            liberarTemp(val);
            return NULL;
        }

        /* operadores compostos */
        case 'a': case 's': case 'm': case 'v': case 'r': {
            const char *op;
            switch(raiz->operador){
                case 'a': op="+"; break; case 's': op="-"; break;
                case 'm': op="*"; break; case 'v': op="/"; break;
                default:  op="%"; break;
            }
            char *rhs = gerarTAC(raiz->direita);
            char *t   = novoTemp();
            printf("%s = %s %s %s\n", t, raiz->esquerda->nome, op, rhs);
            printf("%s = %s\n", raiz->esquerda->nome, t);
            liberarTemp(rhs);
            liberarTemp(t);
            return NULL;
        }

        /* ++ / -- */
        case 'I': {
            char *t = novoTemp();
            printf("%s = %s + 1\n", t, raiz->esquerda->nome);
            printf("%s = %s\n", raiz->esquerda->nome, t);
            liberarTemp(t);
            return NULL;
        }
        case 'D': {
            char *t = novoTemp();
            printf("%s = %s - 1\n", t, raiz->esquerda->nome);
            printf("%s = %s\n", raiz->esquerda->nome, t);
            liberarTemp(t);
            return NULL;
        }

        case ';': {
            liberarTemp(gerarTAC(raiz->esquerda));
            liberarTemp(gerarTAC(raiz->direita));
            return NULL;
        }

        /* if/else */
        case 'f': {
            char *cond = gerarTAC(raiz->esquerda);
            int lf = proximoLabel(), le = proximoLabel();
            char lFalse[16], lEnd[16];
            novoLabel(lFalse, lf); novoLabel(lEnd, le);

            printf("if_false %s goto %s\n", cond, lFalse);
            liberarTemp(cond);

            NoAST *corpo = raiz->direita;
            liberarTemp(gerarTAC(corpo->esquerda));

            if (corpo->direita) {
                printf("goto %s\n", lEnd);
                printf("%s:\n", lFalse);
                liberarTemp(gerarTAC(corpo->direita));
                printf("%s:\n", lEnd);
            } else {
                printf("%s:\n", lFalse);
            }
            return NULL;
        }

        /* while */
        case 'W': {
            int ls = proximoLabel(), le = proximoLabel();
            char lStart[16], lEnd[16];
            novoLabel(lStart, ls); novoLabel(lEnd, le);

            printf("%s:\n", lStart);
            char *cond = gerarTAC(raiz->esquerda);
            printf("if_false %s goto %s\n", cond, lEnd);
            liberarTemp(cond);
            liberarTemp(gerarTAC(raiz->direita));
            printf("goto %s\n", lStart);
            printf("%s:\n", lEnd);
            return NULL;
        }

        /* for */
        case 'F': {
            NoAST *meta  = raiz->esquerda;
            NoAST *resto = raiz->direita;

            liberarTemp(gerarTAC(meta->esquerda));  /* init */

            int ls = proximoLabel(), le = proximoLabel();
            char lStart[16], lEnd[16];
            novoLabel(lStart, ls); novoLabel(lEnd, le);

            printf("%s:\n", lStart);
            if (resto->esquerda) {
                char *cond = gerarTAC(resto->esquerda);
                printf("if_false %s goto %s\n", cond, lEnd);
                liberarTemp(cond);
            }
            liberarTemp(gerarTAC(resto->direita));  /* corpo */
            liberarTemp(gerarTAC(meta->direita));   /* incr */
            printf("goto %s\n", lStart);
            printf("%s:\n", lEnd);
            return NULL;
        }

        /* printf */
        case 'P': {
            char *val = gerarTAC(raiz->esquerda);
            printf("print %s\n", val);
            liberarTemp(val);
            return NULL;
        }

        /* lógicos */
        case 'A': case 'O': {
            char *esq = gerarTAC(raiz->esquerda);
            char *dir = gerarTAC(raiz->direita);
            char *t   = novoTemp();
            printf("%s = %s %s %s\n", t, esq,
                   raiz->operador=='A' ? "&&" : "||", dir);
            liberarTemp(esq); liberarTemp(dir);
            return t;
        }
        case 'N': {
            char *op = gerarTAC(raiz->esquerda);
            char *t  = novoTemp();
            printf("%s = !%s\n", t, op);
            liberarTemp(op);
            return t;
        }

        /* aritméticos e relacionais */
        default: {
            char *esq = gerarTAC(raiz->esquerda);
            char *dir = gerarTAC(raiz->direita);
            char *t   = novoTemp();
            const char *op;
            switch (raiz->operador) {
                case '+': op="+";  break; case '-': op="-";  break;
                case '*': op="*";  break; case '/': op="/";  break;
                case '%': op="%";  break;
                case 'e': op="=="; break; case '!': op="!="; break;
                case '<': op="<";  break; case '>': op=">";  break;
                case 'L': op="<="; break; case 'G': op=">="; break;
                default:  op="?";  break;
            }
            printf("%s = %s %s %s\n", t, esq, op, dir);
            liberarTemp(esq); liberarTemp(dir);
            return t;
        }
    }
}