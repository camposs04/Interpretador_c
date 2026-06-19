# Interpretador de C

## Sobre o Projeto
Este repositório contém o desenvolvimento de um **interpretador da linguagem C**, criado como trabalho em equipe para a disciplina de Compiladores 1 da Universidade de Brasília (UnB).

## Participantes

|            Nome           | Matrícula | 
|---------------------------|-----------|
|José Joaquim da Silva Neto | 232027510 |


# Estrutura do Projeto

```
InterpretadorC/
├── src
│   ├── lexerC.l
│   ├── parserC.y
│   ├── ast.c
│   ├── ast.h
│   ├── tipos.h
│   ├── interpreter.c
│   ├── interpreter.h
│   ├── semantic.c
│   ├── semantic.h
│   ├── tabsym.c
│   ├── tabsym.h
│   ├── tac.c
│   └── tac.h  
├── docs
│   ├── grammar.md
│   ├── lexer.md
│   ├── scope.md
│   └── semantic.md
├── testes
│   ├── overview.md
│   ├── testeAtribuicao.c
│   ├── testeDeclaracoes.C
│   ├── testeErros.c
│   ├── testeEscopo.c
│   ├── testeExpressaoInt.c
│   ├── testeFuncoes.c
│   ├── testeif.c
│   ├── testeMain01.c
│   ├── testeMainLoop.c
│   ├── testePrintf.c
│   ├── testeScanf.c
│   └── testeSimples.c
├── makefile
├── .gitignore
└── README.md
```

# Como executar
## Pré-requisitos
Certifique-se de ter instalado:

No Ubuntu:
```bash
sudo apt update
sudo apt install build-essential flex bison make
```

# Como Complilar e Executar

Para compilar o projeto vá até a raiz do projeto e execute utilizando o Makefile:

### 1.   Compilação padrão
```bash
make all
```
Isso irá gerar o executável `programa` integrando o Léxico (Flex) e o Sintático (Bison).

### 2. Execução
```bash
./programa
```
### 3. Limpeza de arquivos temporários
```bash
make clean
```

Como o `make` já definido, basta rodar no terminal o comando `make all` para uma compilação padrão.

### Diagnóstico de Conflitos

Para desenvolvedores que desejam analisar a integridade da gramática:
- Execute `make examples`.
- Este comando utiliza a flag `-Wcounterexamples` do Bison, que identifica ambiguidades na gramática e imprime no terminal exemplos práticos de caminhos de parsing que geram conflitos (como Shift/Reduce).

# Documentação

A documentação do projeto está no diretório docs, que está estruturado da seguinte forma:

```
docs/
├── grammar.md   
├── lexer.md
├── scope.md
└── semantic.md
```

- `grammar.md`: descreve a gramática da linguagem (análise sintática com Bison)
- `lexer.md`: descreve os tokens e regras léxicas (Flex) s e integração com `yylval`
- `semantic.md`: descreve as ações semânticas e avaliação das expressões