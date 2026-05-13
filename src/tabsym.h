#ifndef TABSYM_H
#define TABSYM_H

typedef struct Symb {
    char *name;
    char *type;
    struct Symb *next;
} Symb;

#define TABLE_SIZE 211 

unsigned int hash(char *str);
void insertSymbol(char *name, char *type);
Symb* searchSymbol(char *name);
void printTable();

#endif