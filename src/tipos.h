typedef enum {
    T_INT,
    T_FLOAT,
    T_CHAR,
    T_BOOL
} Tipo;

typedef struct noAST {
    char operador;

    Tipo tipo;   

    union {
        int i;
        float f;
        char c;
    } valor;

    char nome[32];

    struct noAST *esquerda;
    struct noAST *direita;
} NoAST;