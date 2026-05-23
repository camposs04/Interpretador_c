all:
	bison -d src/parserC.y
	flex src/lexerC.l
	gcc parserC.tab.c lex.yy.c src/ast.c src/tabsym.c src/semantic.c src/tac.c src/interpreter.c -I./src -o programa

example:
	bison -d src/parserC.y -Wcounterexamples
	flex src/lexerC.l
	gcc parserC.tab.c lex.yy.c src/ast.c src/tabsym.c src/semantic.c src/tac.c src/interpreter.c -I./src -o programa

clean:
	rm -f programa parserC.tab.c parserC.tab.h lex.yy.c