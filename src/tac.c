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

static void novoLabel(char *buf, int n) {
    snprintf(buf, 16, "L%d", n);
}

static int proximoLabel(void) {
    return ++contLabel;
}

char *gerarTAC(NoAST *raiz) {
    if (raiz == NULL) return NULL;

    switch (raiz->operador) {

        case 'n': {
            char *t = novoTemp();
            switch (raiz->tipo) {
                case T_INT:
                    printf("%s = %d\n", t, raiz->valor.i);
                    break;
                case T_FLOAT:
                    printf("%s = %f\n", t, raiz->valor.f);
                    break;
                case T_CHAR:
                    printf("%s = '%c'\n", t, raiz->valor.c);
                    break;
                case T_BOOL:
                    printf("%s = %s\n", t, raiz->valor.i ? "true" : "false");
                    break;
            }
            return t;
        }

        case 'i': {
            char *t = malloc(32);
            strncpy(t, raiz->nome, 31);
            t[31] = '\0';
            return t;
        }

        case 'd': {
            char *nome = raiz->esquerda->nome;

            const char *tipo_str;
            switch (raiz->tipo) {
                case T_INT:   tipo_str = "int";   break;
                case T_FLOAT: tipo_str = "float"; break;
                case T_CHAR:  tipo_str = "char";  break;
                case T_BOOL:  tipo_str = "bool";  break;
                default:      tipo_str = "?";     break;
            }

            if (raiz->direita != NULL) {
                char *val = gerarTAC(raiz->direita);
                printf("decl %s %s\n", tipo_str, nome);
                printf("%s = %s\n", nome, val);
                free(val);
            } else {
                printf("decl %s %s\n", tipo_str, nome);
            }
            return NULL;
        }

        case '=': {
            char *val = gerarTAC(raiz->direita);
            printf("%s = %s\n", raiz->esquerda->nome, val);
            free(val);
            return NULL;
        }

        case ';': {
            char *r = gerarTAC(raiz->esquerda);
            free(r);
            r = gerarTAC(raiz->direita);
            free(r);
            return NULL;
        }

        case 'f': {
            char *cond = gerarTAC(raiz->esquerda);

            int labelFalse = proximoLabel();
            int labelEnd   = proximoLabel();

            char lFalse[16], lEnd[16];
            novoLabel(lFalse, labelFalse);
            novoLabel(lEnd,   labelEnd);

            printf("if_false %s goto %s\n", cond, lFalse);
            free(cond);

            NoAST *corpo = raiz->direita;   
            char *r = gerarTAC(corpo->esquerda);
            free(r);

            if (corpo->direita != NULL) {
                /* tem else */
                printf("goto %s\n", lEnd);
                printf("%s:\n", lFalse);
                r = gerarTAC(corpo->direita);
                free(r);
                printf("%s:\n", lEnd);
            } else {
                printf("%s:\n", lFalse);
            }
            return NULL;
        }

        default: {
            char *esq = gerarTAC(raiz->esquerda);
            char *dir = gerarTAC(raiz->direita);
            char *t   = novoTemp();

            const char *op;
            switch (raiz->operador) {
                case '+': op = "+";  break;
                case '-': op = "-";  break;
                case '*': op = "*";  break;
                case '/': op = "/";  break;
                case 'e': op = "=="; break;
                case '!': op = "!="; break;
                case '<': op = "<";  break;
                case '>': op = ">";  break;
                case 'L': op = "<="; break;
                case 'G': op = ">="; break;
                default:  op = "?";  break;
            }

            printf("%s = %s %s %s\n", t, esq, op, dir);
            free(esq);
            free(dir);
            return t;
        }
    }
}