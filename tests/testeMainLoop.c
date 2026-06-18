// Esperado: "0 1 2 3 4" e "positivo"
int main() {
    int i;
    for (i = 0; i < 5; i++) {
        printf("%d ", i);
    }
    printf("\n");

    int x = 10;
    if (x > 0) {
        printf("positivo\n");
    } else {
        printf("negativo\n");
    }

    return 0;
}