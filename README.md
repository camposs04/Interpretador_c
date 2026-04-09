# Interpretador de C

## Sobre o Projeto
Este repositório contém o desenvolvimento de um **interpretador da linguagem C**, criado como trabalho em equipe para a disciplina de Compiladores 1 da Universidade de Brasília (UnB).

# Estrutura do Projeto

```
InterpretadorC/
├── src
│   ├── lexerC.l
│   └── parserC.y   
├── docs
│   ├── grammar.md
│   ├── lexer.md
│   └── semantic.md
├── testes
│   └── overview.md
├── makefile
└── README.md
```

# Como executar
## Pré-requisitos
Certifique-se de ter instalado:

No Ubuntu:
```
sudo apt update
sudo apt install build-essential flex bison make
```

# Compilação

Para compilar o projeto vá até a raiz do projeto e execute:
```
make
```
Após isso execute o arquivo:
```
./programa
```

# Documentação

A documentação do projeto está no diretório docs, que está estruturado da seguinte forma:

```
docs/
├── grammar.md   
├── lexer.md
└── semantic.md
```

- grammar.md: descreve a gramática da linguagem (análise sintática com Bison)
- lexer.md: descreve os tokens e regras léxicas (Flex)
- semantic.md: descreve as ações semânticas e avaliação das expressões