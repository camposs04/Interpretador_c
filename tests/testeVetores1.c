// ===========================================================
// Teste: vetores (declaracao, leitura, escrita)
// Programa VALIDO (nenhum erro semantico esperado).
//
// Saida esperada:
//   1.500000 2.500000 3.500000
//   0 1 4 9 16
//   soma=7.000000
//   v0=-3 v1=3 fib7=13
// ===========================================================

float w[3] = {1.5, 2.5, 3.5};
printf("%f %f %f\n", w[0], w[1], w[2]);

int v[5];
int i = 0;
while (i < 5) {
    v[i] = i * i;
    i = i + 1;
}
i = 0;
while (i < 5) {
    printf("%d ", v[i]);
    i = i + 1;
}
printf("\n");

float soma = 0.0;
float w2[3] = {1.5, 2.5, 3.0};
int j = 0;
for (j = 0; j < 3; j++) {
    soma += w2[j];
}
printf("soma=%f\n", soma);

int fib(int n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
}
int u[5];
u[0] = -3;
u[1] = -u[0];
printf("v0=%d v1=%d fib7=%d\n", u[0], u[1], fib(7));