all:
	bison -d src/parserC.y
	flex src/lexerC.l
	gcc parserC.tab.c lex.yy.c src/ast.c -I./src -o programa

example:
	bison -d src/parserC.y -Wcounterexamples
	flex src/lexerC.l
	gcc parserC.tab.c lex.yy.c src/ast.c -I./src -o programa
        
clean:
	rm -f programa parserC.tab.c parserC.tab.h lex.yy.c