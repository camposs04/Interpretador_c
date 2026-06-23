#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tabsym.h"
#include "ast.h"
#include "semantic.h"

static int numErros = 0;
static int profundidadeLaco = 0;
static Tipo tipoFuncaoAtual = T_VOID;
static int dentroDeFuncao = 0;

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

/* Detecta o uso do retorno de uma função void como se fosse um valor
   (ex.: "x = minhaFuncaoVoid();", "1 + minhaFuncaoVoid()"). Deve ser chamada
   APÓS a expressão já ter sido analisada (para s->retorno já estar resolvido). */
static void verificarNaoVoid(NoAST *expr) {
    if (expr == NULL || expr->operador != 'C') return;
    Symb *s = searchSymbol(expr->nome);
    if (s && s->isFuncao && s->retorno == T_VOID) {
        printf("Erro Semantico: funcao '%s' tem retorno void e nao pode ser usada como valor.\n",
               expr->nome);
        numErros++;
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

    if (raiz->direita) {
        analisarSemantica(raiz->direita);
        verificarNaoVoid(raiz->direita);
    }
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

        /* ── declaração de vetor ── */
        case 'V': {
            const char *tipo_str = tipoStr(raiz->tipo);

            if (searchSymbolEscopoAtual(raiz->nome) != NULL) {
                printf("Erro Semantico: variavel '%s' ja declarada neste escopo.\n", raiz->nome);
                numErros++;
                return;
            }
            if (raiz->valor.i <= 0) {
                printf("Erro Semantico: vetor '%s' deve ter tamanho positivo.\n", raiz->nome);
                numErros++;
                return;
            }
            insertVetor(raiz->nome, tipo_str, raiz->valor.i);

            /* Verifica a lista de valores iniciais, se houver */
            if (raiz->direita) {
                int nvalores = 0;
                NoAST *cur = raiz->direita;
                while (cur != NULL) {
                    if (cur->operador == 'L') {
                        analisarSemantica(cur->esquerda);
                        verificarNaoVoid(cur->esquerda);
                        nvalores++;
                        cur = cur->direita;
                    } else {
                        analisarSemantica(cur);
                        verificarNaoVoid(cur);
                        nvalores++;
                        break;
                    }
                }
                if (nvalores > raiz->valor.i) {
                    printf("Erro Semantico: vetor '%s' tem %d valor(es) inicial(is) para %d posicao(oes).\n",
                           raiz->nome, nvalores, raiz->valor.i);
                    numErros++;
                }
            }
            return;
        }

        case 'i':
            identifier(raiz);
            return;

        /* ── acesso de leitura a vetor (vetor[i]) ── */
        case 'X': {
            Symb *s = searchSymbol(raiz->nome);
            if (!s || !s->isVetor) {
                printf("Erro Semantico: '%s' nao e um vetor declarado.\n", raiz->nome);
                numErros++;
                return;
            }
            analisarSemantica(raiz->esquerda); /* índice */
            verificarNaoVoid(raiz->esquerda);
            if      (strcmp(s->type,"int")   == 0) raiz->tipo = T_INT;
            else if (strcmp(s->type,"float") == 0) raiz->tipo = T_FLOAT;
            else if (strcmp(s->type,"char")  == 0) raiz->tipo = T_CHAR;
            else if (strcmp(s->type,"bool")  == 0) raiz->tipo = T_BOOL;
            return;
        }

        /* ── atribuição a elemento de vetor (vetor[i] = valor;) ── */
        case 'Y': {
            Symb *s = searchSymbol(raiz->nome);
            if (!s || !s->isVetor) {
                printf("Erro Semantico: '%s' nao e um vetor declarado.\n", raiz->nome);
                numErros++;
                return;
            }
            analisarSemantica(raiz->esquerda); /* índice */
            analisarSemantica(raiz->direita);  /* valor */
            verificarNaoVoid(raiz->esquerda);
            verificarNaoVoid(raiz->direita);
            return;
        }

        /* atribuição e operadores compostos */
        case '=':
        case 'a': case 's': case 'm': case 'v': case 'r':
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            verificarNaoVoid(raiz->direita);
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
            profundidadeLaco++;
            analisarSemantica(raiz->direita);
            profundidadeLaco--;
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
            profundidadeLaco++;
            analisarSemantica(resto->direita);
            profundidadeLaco--;
            sairEscopo();
            sairEscopo();
            return;
        }

        /* ── break ── */
        case 'B':
            if (profundidadeLaco == 0) {
                printf("Erro Semantico: 'break' usado fora de um laco.\n");
                numErros++;
            }
            return;

        /* ── bloco solto: precisa de escopo próprio ── */
        case 'Q':
            entrarEscopo();
            analisarSemantica(raiz->esquerda);
            sairEscopo();
            return;

        case 'P':
            analisarSemantica(raiz->esquerda);
            verificarNaoVoid(raiz->esquerda);
            return;

        case 'R': {
            NoAST *cur = raiz->direita;
            while (cur != NULL) {
                if (cur->operador == 'L') {
                    analisarSemantica(cur->esquerda);
                    verificarNaoVoid(cur->esquerda);
                    cur = cur->direita;
                } else {
                    analisarSemantica(cur);
                    verificarNaoVoid(cur);
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

        case 'u':
            analisarSemantica(raiz->esquerda);
            raiz->tipo = raiz->esquerda->tipo;
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

            /* Salva contexto da função externa (suporta funções aninhadas,
               mesmo que a gramática atual não as produza) para checar os
               'return' do corpo contra o tipo de retorno declarado. */
            Tipo tipoAnterior   = tipoFuncaoAtual;
            int  dentroAnterior = dentroDeFuncao;
            tipoFuncaoAtual  = raiz->tipo;
            dentroDeFuncao   = 1;

            analisarSemantica(raiz->esquerda);

            tipoFuncaoAtual = tipoAnterior;
            dentroDeFuncao  = dentroAnterior;

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
                    verificarNaoVoid(cur->esquerda);
                    cur = cur->direita;
                } else {
                    analisarSemantica(cur);
                    verificarNaoVoid(cur);
                    break;
                }
            }
            return;
        }

        /* ── return ── */
        case 'K':
            analisarSemantica(raiz->esquerda);
            verificarNaoVoid(raiz->esquerda);

            if (!dentroDeFuncao) {
                /* return no nível global: sem tipo de função pra validar,
                   apenas deixa passar (comportamento já existente). */
                return;
            }
            if (tipoFuncaoAtual == T_VOID) {
                if (raiz->esquerda != NULL) {
                    printf("Erro Semantico: funcao void nao pode retornar um valor.\n");
                    numErros++;
                }
            } else {
                if (raiz->esquerda == NULL) {
                    printf("Erro Semantico: funcao do tipo '%s' deve retornar um valor.\n",
                           tipoStr(tipoFuncaoAtual));
                    numErros++;
                }
            }
            return;

        default:
            analisarSemantica(raiz->esquerda);
            analisarSemantica(raiz->direita);
            verificarNaoVoid(raiz->esquerda);
            verificarNaoVoid(raiz->direita);
            if (raiz->esquerda && raiz->direita) {
                Tipo te = raiz->esquerda->tipo;
                Tipo td = raiz->direita->tipo;
                raiz->tipo = (te == T_FLOAT || td == T_FLOAT) ? T_FLOAT : T_INT;
            }
            return;
    }
}