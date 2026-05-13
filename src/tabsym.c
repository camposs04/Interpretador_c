#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabsym.h"

Symb *simbltable[TABLE_SIZE] = {NULL};

unsigned int hash(char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash += *str++;
    }
    return hash % TABLE_SIZE;
}

Symb* searchSymbol(char *name) {
    unsigned int i = hash(name);
    Symb *current = simbltable[i];

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void insertSymbol(char *name, char *type) {
    if (searchSymbol(name) != NULL) {
        printf("Error: Symbol '%s' already exists.\n", name);
        return;
    }

    unsigned int i = hash(name);
    Symb *new = (Symb*) malloc(sizeof(Symb));
    new->name = strdup(name);
    new->type = strdup(type);

    new->next = simbltable[i]; 
    simbltable[i] = new;
}

void imprimirTabela() {
    printf("\n--- SYMBOL TABLE ---\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (simbltable[i] != NULL) {
            Symb *current = simbltable[i];
            printf("Bucket [%d]: ", i);
            while (current != NULL) {
                printf("(%s, %s) -> ", current->name, current->type);
                current = current->next;
            }
            printf("NULL\n");
        }
    }
}