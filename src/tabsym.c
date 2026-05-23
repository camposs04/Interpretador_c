#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabsym.h"

static Escopo *escopoAtual = NULL;

static unsigned int hash(const char *str) {
    unsigned int h = 0;
    while (*str) {
        h += *str++;
    }
    return h % TABLE_SIZE;
}

void entrarEscopo(void) {
    Escopo *novo = malloc(sizeof(Escopo));
    for (int i = 0; i < TABLE_SIZE; i++)
        novo->tabela[i] = NULL;
    novo->anterior = escopoAtual;
    escopoAtual = novo;
}

void sairEscopo(void) {
    if (escopoAtual == NULL) return;

    /* libera os símbolos do escopo atual */
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symb *s = escopoAtual->tabela[i];
        while (s != NULL) {
            Symb *prox = s->next;
            free(s->name);
            free(s->type);
            free(s);
            s = prox;
        }
    }

    Escopo *anterior = escopoAtual->anterior;
    free(escopoAtual);
    escopoAtual = anterior;
}

/* busca só no escopo atual — usada para detectar redeclaração */
Symb* searchSymbolEscopoAtual(const char *name) {
    if (escopoAtual == NULL) return NULL;

    unsigned int i = hash(name);
    Symb *s = escopoAtual->tabela[i];
    while (s != NULL) {
        if (strcmp(s->name, name) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}

/* busca em todos os escopos — usada para detectar uso sem declaração */
Symb* searchSymbol(const char *name) {
    Escopo *e = escopoAtual;
    while (e != NULL) {
        unsigned int i = hash(name);
        Symb *s = e->tabela[i];
        while (s != NULL) {
            if (strcmp(s->name, name) == 0)
                return s;
            s = s->next;
        }
        e = e->anterior;
    }
    return NULL;
}

void insertSymbol(const char *name, const char *type) {
    if (escopoAtual == NULL) {
        printf("Erro interno: nenhum escopo ativo.\n");
        return;
    }

    unsigned int i = hash(name);
    Symb *novo = malloc(sizeof(Symb));
    novo->name = strdup(name);
    novo->type = strdup(type);
    novo->next = escopoAtual->tabela[i];
    escopoAtual->tabela[i] = novo;
}

void imprimirTabela(void) {
    printf("\n--- SYMBOL TABLE ---\n");
    Escopo *e = escopoAtual;
    int nivel = 0;
    while (e != NULL) {
        printf("Escopo [%d]:\n", nivel++);
        for (int i = 0; i < TABLE_SIZE; i++) {
            Symb *s = e->tabela[i];
            if (s != NULL) {
                printf("  Bucket [%d]: ", i);
                while (s != NULL) {
                    printf("(%s, %s) -> ", s->name, s->type);
                    s = s->next;
                }
                printf("NULL\n");
            }
        }
        e = e->anterior;
    }
}