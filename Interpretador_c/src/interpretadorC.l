%{
#include "teste.tab.h"
#include <stdio.h>
%}

%%

"print"     {return PRINT; }

[ \t\n]+    { /* ignora espaços e quebras de linha */ }

.           { /* ignora qualquer outro caractere */ }

%%

int yywrap(void) {
    return 1;
}

int main(int argc, char **argv) {
    return yyparse();
}