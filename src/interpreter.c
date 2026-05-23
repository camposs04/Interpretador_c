#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "ast.h"

/* ── pilha de escopos em tempo de execução ── */
static EscopoRT *escopoAtual = NULL;

static void entrarEscopoRT(void) {
    EscopoRT *novo = malloc(sizeof(EscopoRT));
    novo->vars     = NULL;
    novo->anterior = escopoAtual;
    escopoAtual    = novo;
}

static void sairEscopoRT(void) {
    if (escopoAtual == NULL) return;

    VarRT *v = escopoAtual->vars;
    while (v != NULL) {
        VarRT *prox = v->prox;
        free(v);
        v = prox;
    }

    EscopoRT *ant = escopoAtual->anterior;
    free(escopoAtual);
    escopoAtual = ant;
}

/* ── acesso a variáveis ── */
static VarRT *buscarVar(char *nome) {
    EscopoRT *e = escopoAtual;
    while (e != NULL) {
        VarRT *v = e->vars;
        while (v != NULL) {
            if (strcmp(v->nome, nome) == 0) return v;
            v = v->prox;
        }
        e = e->anterior;
    }
    return NULL;
}

static VarRT *declararVar(char *nome, Tipo tipo) {
    VarRT *v = malloc(sizeof(VarRT));
    strncpy(v->nome, nome, 31);
    v->nome[31] = '\0';
    v->valor.tipo   = tipo;
    v->valor.dado.i = 0;
    v->prox         = escopoAtual->vars;
    escopoAtual->vars = v;
    return v;
}

/* ── impressão de valor ── */
static void imprimirValor(Valor v) {
    switch (v.tipo) {
        case T_INT:   printf("%d", v.dado.i);              break;
        case T_FLOAT: printf("%g", v.dado.f);              break;
        case T_CHAR:  printf("%c", v.dado.c);              break;
        case T_BOOL:  printf("%s", v.dado.i ? "true" : "false"); break;
    }
}

/* ── avaliação de expressão ── */
static Valor avaliar(NoAST *raiz) {
    Valor resultado;
    resultado.tipo   = T_INT;
    resultado.dado.i = 0;

    if (raiz == NULL) return resultado;

    switch (raiz->operador) {

        /* literal */
        case 'n':
            resultado.tipo = raiz->tipo;
            switch (raiz->tipo) {
                case T_INT:   resultado.dado.i = raiz->valor.i; break;
                case T_FLOAT: resultado.dado.f = raiz->valor.f; break;
                case T_CHAR:  resultado.dado.c = raiz->valor.c; break;
                case T_BOOL:  resultado.dado.i = raiz->valor.i; break;
            }
            return resultado;

        /* identificador */
        case 'i': {
            VarRT *v = buscarVar(raiz->nome);
            if (v == NULL) {
                printf("Erro em tempo de execucao: variavel '%s' nao encontrada.\n", raiz->nome);
                return resultado;
            }
            return v->valor;
        }

        /* operações aritméticas e relacionais */
        default: {
            Valor esq = avaliar(raiz->esquerda);
            Valor dir = avaliar(raiz->direita);

            /* promove para float se necessário */
            int usaFloat = (esq.tipo == T_FLOAT || dir.tipo == T_FLOAT);

            float ve = (esq.tipo == T_FLOAT) ? esq.dado.f : (float)esq.dado.i;
            float vd = (dir.tipo == T_FLOAT) ? dir.dado.f : (float)dir.dado.i;

            switch (raiz->operador) {
                case '+':
                    if (usaFloat) { resultado.tipo = T_FLOAT; resultado.dado.f = ve + vd; }
                    else          { resultado.tipo = T_INT;   resultado.dado.i = esq.dado.i + dir.dado.i; }
                    break;
                case '-':
                    if (usaFloat) { resultado.tipo = T_FLOAT; resultado.dado.f = ve - vd; }
                    else          { resultado.tipo = T_INT;   resultado.dado.i = esq.dado.i - dir.dado.i; }
                    break;
                case '*':
                    if (usaFloat) { resultado.tipo = T_FLOAT; resultado.dado.f = ve * vd; }
                    else          { resultado.tipo = T_INT;   resultado.dado.i = esq.dado.i * dir.dado.i; }
                    break;
                case '/':
                    if (vd == 0) {
                        printf("Erro em tempo de execucao: divisao por zero.\n");
                        return resultado;
                    }
                    if (usaFloat) { resultado.tipo = T_FLOAT; resultado.dado.f = ve / vd; }
                    else          { resultado.tipo = T_INT;   resultado.dado.i = esq.dado.i / dir.dado.i; }
                    break;
                /* relacionais — resultado sempre T_BOOL (int 0/1) */
                case 'e': resultado.tipo = T_BOOL; resultado.dado.i = (ve == vd); break;
                case '!': resultado.tipo = T_BOOL; resultado.dado.i = (ve != vd); break;
                case '<': resultado.tipo = T_BOOL; resultado.dado.i = (ve <  vd); break;
                case '>': resultado.tipo = T_BOOL; resultado.dado.i = (ve >  vd); break;
                case 'L': resultado.tipo = T_BOOL; resultado.dado.i = (ve <= vd); break;
                case 'G': resultado.tipo = T_BOOL; resultado.dado.i = (ve >= vd); break;
                default:
                    printf("Erro: operador '%c' desconhecido.\n", raiz->operador);
                    break;
            }
            return resultado;
        }
    }
}

/* ── execução de comandos ── */
static void executar(NoAST *raiz) {
    if (raiz == NULL) return;

    switch (raiz->operador) {

        /* sequência */
        case ';':
            executar(raiz->esquerda);
            executar(raiz->direita);
            break;

        /* declaração */
        case 'd': {
            char *nome = raiz->esquerda->nome;
            VarRT *v   = declararVar(nome, raiz->tipo);

            if (raiz->direita != NULL) {
                Valor val = avaliar(raiz->direita);
                v->valor  = val;
                v->valor.tipo = raiz->tipo;
            }

            printf("[decl] %s ", nome);
            imprimirValor(v->valor);
            printf("\n");
            break;
        }

        /* atribuição */
        case '=': {
            char  *nome = raiz->esquerda->nome;
            Valor  val  = avaliar(raiz->direita);
            VarRT *v    = buscarVar(nome);

            if (v == NULL) {
                printf("Erro em tempo de execucao: variavel '%s' nao encontrada.\n", nome);
                break;
            }

            /* converte se necessário */
            if (v->valor.tipo == T_FLOAT && val.tipo == T_INT) {
                val.tipo   = T_FLOAT;
                val.dado.f = (float)val.dado.i;
            } else if (v->valor.tipo == T_INT && val.tipo == T_FLOAT) {
                val.tipo   = T_INT;
                val.dado.i = (int)val.dado.f;
            }

            v->valor = val;

            printf("[atrib] %s = ", nome);
            imprimirValor(v->valor);
            printf("\n");
            break;
        }

        /* if/else */
        case 'f': {
            Valor cond = avaliar(raiz->esquerda);
            NoAST *corpo = raiz->direita;

            if (cond.dado.i) {
                entrarEscopoRT();
                executar(corpo->esquerda);
                sairEscopoRT();
            } else if (corpo->direita != NULL) {
                entrarEscopoRT();
                executar(corpo->direita);
                sairEscopoRT();
            }
            break;
        }

        default:
            /* expressão solta — avalia e descarta */
            avaliar(raiz);
            break;
    }
}

/* ── ponto de entrada público ── */
void interpretarPrograma(NoAST *raiz) {
    entrarEscopoRT();
    printf("\n--- EXECUCAO ---\n");
    executar(raiz);
    printf("--- FIM ---\n");
    sairEscopoRT();
}