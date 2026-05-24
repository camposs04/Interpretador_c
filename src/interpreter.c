#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "ast.h"

extern int debug;

/* ── pilha de escopos em tempo de execução ── */
static EscopoRT *escopoAtual = NULL;

static void entrarEscopoRT(void) {
    EscopoRT *novo  = malloc(sizeof(EscopoRT));
    novo->vars      = NULL;
    novo->anterior  = escopoAtual;
    escopoAtual     = novo;
}

static void sairEscopoRT(void) {
    if (escopoAtual == NULL) return;
    VarRT *v = escopoAtual->vars;
    while (v) { VarRT *p = v->prox; free(v); v = p; }
    EscopoRT *ant = escopoAtual->anterior;
    free(escopoAtual);
    escopoAtual = ant;
}

/* ── acesso a variáveis ── */
static VarRT *buscarVar(const char *nome) {
    for (EscopoRT *e = escopoAtual; e; e = e->anterior)
        for (VarRT *v = e->vars; v; v = v->prox)
            if (strcmp(v->nome, nome) == 0) return v;
    return NULL;
}

static VarRT *declararVar(const char *nome, Tipo tipo) {
    VarRT *v = calloc(1, sizeof(VarRT));
    strncpy(v->nome, nome, 31);
    v->valor.tipo   = tipo;
    v->valor.dado.i = 0;
    v->prox         = escopoAtual->vars;
    escopoAtual->vars = v;
    return v;
}

/* ── utilitários de valor ── */
static void imprimirValor(Valor v) {
    switch (v.tipo) {
        case T_INT:   printf("%d",  v.dado.i); break;
        case T_FLOAT: printf("%g",  v.dado.f); break;
        case T_CHAR:  printf("%c",  v.dado.c); break;
        case T_BOOL:  printf("%s",  v.dado.i ? "true" : "false"); break;
        default: break;
    }
}

/* converte Valor para float para operações mistas */
static float toFloat(Valor v) {
    return (v.tipo == T_FLOAT) ? v.dado.f : (float)v.dado.i;
}

/* converte Valor para int (verdade) para condicionais */
static int toInt(Valor v) {
    return (v.tipo == T_FLOAT) ? (int)v.dado.f : v.dado.i;
}

/* aplica conversão de tipo ao atribuir */
static Valor converterPara(Valor val, Tipo destino) {
    if (val.tipo == destino) return val;
    Valor r; r.tipo = destino;
    switch (destino) {
        case T_INT:
            r.dado.i = (val.tipo == T_FLOAT) ? (int)val.dado.f : val.dado.i;
            break;
        case T_FLOAT:
            r.dado.f = (val.tipo == T_INT || val.tipo == T_BOOL)
                       ? (float)val.dado.i : val.dado.f;
            break;
        case T_CHAR:
            r.dado.c = (char)((val.tipo == T_FLOAT) ? (int)val.dado.f : val.dado.i);
            break;
        case T_BOOL:
            r.dado.i = toInt(val) ? 1 : 0;
            break;
        default:
            r.dado.i = 0;
    }
    return r;
}

/* ── avaliação de expressão ── */
static Valor avaliar(NoAST *raiz) {
    Valor resultado = { T_INT, {0} };
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
                default: break;
            }
            return resultado;

        /* identificador */
        case 'i': {
            VarRT *v = buscarVar(raiz->nome);
            if (!v) {
                printf("Erro RT: variavel '%s' nao encontrada.\n", raiz->nome);
                return resultado;
            }
            return v->valor;
        }

        /* lógicos */
        case 'A': {
            Valor esq = avaliar(raiz->esquerda);
            /* curto-circuito */
            if (!toInt(esq)) { resultado.tipo = T_BOOL; resultado.dado.i = 0; return resultado; }
            Valor dir = avaliar(raiz->direita);
            resultado.tipo = T_BOOL;
            resultado.dado.i = toInt(dir) ? 1 : 0;
            return resultado;
        }
        case 'O': {
            Valor esq = avaliar(raiz->esquerda);
            if (toInt(esq))  { resultado.tipo = T_BOOL; resultado.dado.i = 1; return resultado; }
            Valor dir = avaliar(raiz->direita);
            resultado.tipo = T_BOOL;
            resultado.dado.i = toInt(dir) ? 1 : 0;
            return resultado;
        }
        case 'N': {
            Valor op = avaliar(raiz->esquerda);
            resultado.tipo   = T_BOOL;
            resultado.dado.i = toInt(op) ? 0 : 1;
            return resultado;
        }

        /* aritméticos e relacionais */
        default: {
            Valor esq = avaliar(raiz->esquerda);
            Valor dir = avaliar(raiz->direita);
            int usaFloat = (esq.tipo == T_FLOAT || dir.tipo == T_FLOAT);
            float ve = toFloat(esq), vd = toFloat(dir);

            switch (raiz->operador) {
                case '+':
                    if (usaFloat) { resultado.tipo=T_FLOAT; resultado.dado.f=ve+vd; }
                    else          { resultado.tipo=T_INT;   resultado.dado.i=esq.dado.i+dir.dado.i; }
                    break;
                case '-':
                    if (usaFloat) { resultado.tipo=T_FLOAT; resultado.dado.f=ve-vd; }
                    else          { resultado.tipo=T_INT;   resultado.dado.i=esq.dado.i-dir.dado.i; }
                    break;
                case '*':
                    if (usaFloat) { resultado.tipo=T_FLOAT; resultado.dado.f=ve*vd; }
                    else          { resultado.tipo=T_INT;   resultado.dado.i=esq.dado.i*dir.dado.i; }
                    break;
                case '/':
                    if (vd == 0.0f) {
                        printf("Erro RT: divisao por zero.\n");
                        return resultado;
                    }
                    if (usaFloat) { resultado.tipo=T_FLOAT; resultado.dado.f=ve/vd; }
                    else          { resultado.tipo=T_INT;   resultado.dado.i=esq.dado.i/dir.dado.i; }
                    break;
                case '%':
                    if (dir.dado.i == 0) { printf("Erro RT: modulo por zero.\n"); return resultado; }
                    resultado.tipo=T_INT; resultado.dado.i=esq.dado.i % dir.dado.i;
                    break;
                case 'e': resultado.tipo=T_BOOL; resultado.dado.i=(ve==vd); break;
                case '!': resultado.tipo=T_BOOL; resultado.dado.i=(ve!=vd); break;
                case '<': resultado.tipo=T_BOOL; resultado.dado.i=(ve< vd); break;
                case '>': resultado.tipo=T_BOOL; resultado.dado.i=(ve> vd); break;
                case 'L': resultado.tipo=T_BOOL; resultado.dado.i=(ve<=vd); break;
                case 'G': resultado.tipo=T_BOOL; resultado.dado.i=(ve>=vd); break;
                default:
                    printf("Erro RT: operador '%c' desconhecido.\n", raiz->operador);
                    break;
            }
            return resultado;
        }
    }
}

/* ── execução de comandos — forward declaration ── */
static void executar(NoAST *raiz);

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
            const char *nome = raiz->esquerda->nome;
            VarRT *v = declararVar(nome, raiz->tipo);
            if (raiz->direita) {
                Valor val = avaliar(raiz->direita);
                v->valor  = converterPara(val, raiz->tipo);
            }
            if (debug) { printf("[decl] %s = ", nome); imprimirValor(v->valor); printf("\n"); }
            break;
        }

        /* atribuição simples */
        case '=': {
            const char *nome = raiz->esquerda->nome;
            VarRT *v = buscarVar(nome);
            if (!v) { printf("Erro RT: variavel '%s' nao encontrada.\n", nome); break; }
            Valor val = avaliar(raiz->direita);
            v->valor  = converterPara(val, v->valor.tipo);
            if (debug) { printf("[atrib] %s = ", nome); imprimirValor(v->valor); printf("\n"); }
            break;
        }

        /* operadores compostos: nó '=' com filho especial '+=' etc.
           O parser gera esses como atribuições expandidas, mas caso
           venham como operador próprio ('a','s','m','v','r') tratamos aqui. */
        case 'a': /* += */
        case 's': /* -= */
        case 'm': /* *= */
        case 'v': /* /= */
        case 'r': /* %= */
        {
            const char *nome = raiz->esquerda->nome;
            VarRT *v = buscarVar(nome);
            if (!v) { printf("Erro RT: variavel '%s' nao encontrada.\n", nome); break; }
            Valor esq = v->valor;
            Valor dir = avaliar(raiz->direita);
            Valor res = { T_INT, {0} };
            int usaFloat = (esq.tipo==T_FLOAT || dir.tipo==T_FLOAT);
            float ve = toFloat(esq), vd = toFloat(dir);
            switch (raiz->operador) {
                case 'a':
                    if(usaFloat){res.tipo=T_FLOAT;res.dado.f=ve+vd;}
                    else{res.tipo=T_INT;res.dado.i=esq.dado.i+dir.dado.i;}
                    break;
                case 's':
                    if(usaFloat){res.tipo=T_FLOAT;res.dado.f=ve-vd;}
                    else{res.tipo=T_INT;res.dado.i=esq.dado.i-dir.dado.i;}
                    break;
                case 'm':
                    if(usaFloat){res.tipo=T_FLOAT;res.dado.f=ve*vd;}
                    else{res.tipo=T_INT;res.dado.i=esq.dado.i*dir.dado.i;}
                    break;
                case 'v':
                    if(vd==0.0f){printf("Erro RT: divisao por zero.\n");break;}
                    if(usaFloat){res.tipo=T_FLOAT;res.dado.f=ve/vd;}
                    else{res.tipo=T_INT;res.dado.i=esq.dado.i/dir.dado.i;}
                    break;
                case 'r':
                    if(dir.dado.i==0){printf("Erro RT: modulo por zero.\n");break;}
                    res.tipo=T_INT; res.dado.i=esq.dado.i%dir.dado.i;
                    break;
            }
            v->valor = converterPara(res, v->valor.tipo);
            if (debug) { printf("[op=] %s = ", nome); imprimirValor(v->valor); printf("\n"); }
            break;
        }

        /* ++ e -- (pré/pós não distinguidos — efeito colateral é o mesmo) */
        case 'I': /* ++ */
        case 'D': /* -- */
        {
            /* filho esquerdo é nó 'i' com o nome da variável */
            const char *nome = (raiz->esquerda) ? raiz->esquerda->nome : raiz->nome;
            VarRT *v = buscarVar(nome);
            if (!v) { printf("Erro RT: variavel '%s' nao encontrada.\n", nome); break; }
            if (v->valor.tipo == T_FLOAT)
                v->valor.dado.f += (raiz->operador == 'I') ? 1.0f : -1.0f;
            else
                v->valor.dado.i += (raiz->operador == 'I') ? 1 : -1;
            if (debug) { printf("[%s] %s = ", raiz->operador=='I' ? "++" : "--", nome); imprimirValor(v->valor); printf("\n"); }
            break;
        }

        /* if / else */
        case 'f': {
            Valor cond  = avaliar(raiz->esquerda);
            NoAST *corpo = raiz->direita;
            if (toInt(cond)) {
                entrarEscopoRT();
                executar(corpo->esquerda);
                sairEscopoRT();
            } else if (corpo->direita) {
                entrarEscopoRT();
                executar(corpo->direita);
                sairEscopoRT();
            }
            break;
        }

        /* while */
        case 'W': {
            while (1) {
                Valor cond = avaliar(raiz->esquerda);
                if (!toInt(cond)) break;
                entrarEscopoRT();
                executar(raiz->direita);
                sairEscopoRT();
            }
            break;
        }

        /* for: esquerda = meta(init, incr), direita = seq(cond, corpo) */
        case 'F': {
            NoAST *meta  = raiz->esquerda;
            NoAST *resto = raiz->direita;
            entrarEscopoRT();            /* escopo do for */
            executar(meta->esquerda);    /* init */
            while (1) {
                if (resto->esquerda) {
                    Valor cond = avaliar(resto->esquerda);
                    if (!toInt(cond)) break;
                }
                entrarEscopoRT();
                executar(resto->direita);  /* corpo */
                sairEscopoRT();
                executar(meta->direita);   /* incr */
            }
            sairEscopoRT();
            break;
        }

        /* printf simples: imprime o valor da expressão (forma antiga) */
        case 'P': {
            Valor val = avaliar(raiz->esquerda);
            imprimirValor(val);
            printf("\n");
            break;
        }

        /* printf com string de formato: printf("fmt", arg1, arg2, ...) */
        /* printf com formato: "R" */
        case 'R': {
            const char *fmt = raiz->esquerda->nome;

            /* a lista de args usa operador 'L', construída ao contrário pelo parser
               (left-recursive): L(c, L(b, L(a, NULL))) → percorrendo: c,b,a
               então coletamos em pilha e invertemos para obter a,b,c */
            Valor pilha[32];
            int   np = 0;
            NoAST *cur = raiz->direita;
            while (cur != NULL && np < 32) {
                if (cur->operador == 'L') {
                    pilha[np++] = avaliar(cur->esquerda);
                    cur = cur->direita;
                } else {
                    pilha[np++] = avaliar(cur);
                    break;
                }
            }
            /* inverte para ordem de declaração */
            Valor args[32];
            int   nargs = np;
            for (int ii = 0; ii < np; ii++)
                args[ii] = pilha[np - 1 - ii];

            /* percorre a string de formato */
            int ai = 0;
            for (int fi = 0; fmt[fi] != '\0'; fi++) {
                if (fmt[fi] == '\\') {
                    fi++;
                    switch (fmt[fi]) {
                        case 'n':  putchar('\n'); break;
                        case 't':  putchar('\t'); break;
                        case 'r':  putchar('\r'); break;
                        case '\\': putchar('\\'); break;
                        case '"':  putchar('"');  break;
                        default:   putchar('\\'); putchar(fmt[fi]); break;
                    }
                } else if (fmt[fi] == '%') {
                    fi++;
                    if (ai >= nargs) { putchar('%'); putchar(fmt[fi]); continue; }
                    Valor v = args[ai++];
                    switch (fmt[fi]) {
                        case 'd': case 'i':
                            printf("%d", (v.tipo == T_FLOAT) ? (int)v.dado.f : v.dado.i);
                            break;
                        case 'f':
                            printf("%f", (v.tipo == T_FLOAT) ? v.dado.f : (float)v.dado.i);
                            break;
                        case 'g':
                            printf("%g", (v.tipo == T_FLOAT) ? v.dado.f : (float)v.dado.i);
                            break;
                        case 'c':
                            printf("%c", (v.tipo == T_CHAR) ? v.dado.c : (char)v.dado.i);
                            break;
                        case 's':
                            if (v.tipo == T_BOOL)
                                printf("%s", v.dado.i ? "true" : "false");
                            else
                                printf("%d", v.dado.i);
                            break;
                        case '%': putchar('%'); ai--; break;
                        default:  putchar('%'); putchar(fmt[fi]); ai--; break;
                    }
                } else {
                    putchar(fmt[fi]);
                }
            }
            break;
        }

        default:
            /* expressão solta (e.g. chamada de função futura) */
            avaliar(raiz);
            break;
    }
}

/* ── ponto de entrada público ── */
void interpretarPrograma(NoAST *raiz) {
    entrarEscopoRT();
    if (debug) printf("\n--- EXECUCAO ---\n");
    executar(raiz);
    if (debug) printf("--- FIM ---\n");
    sairEscopoRT();
}