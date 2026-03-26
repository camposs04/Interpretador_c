%{
#include <stdio.h>
#include <stdHelib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);

%}

%token PRINT

%%

    PRINT {printf("Prin funcionando!\n"); }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}