#ifndef TABSYM_H
#define TABSYM_H

#define TABLE_SIZE 211 

typedef struct Symb {
    char *name;
    char *type;
    struct Symb *next;
} Symb;

typedef struct Escopo {
    Symb *tabela[TABLE_SIZE];
    struct Escopo *anterior;
} Escopo;

void entrarEscopo(void);
void sairEscopo(void);
void insertSymbol(char *name, char *type);
Symb* searchSymbol(char *name);
Symb* searchSymbolEscopoAtual(char *name);
void imprimirTabela(void);

#endif