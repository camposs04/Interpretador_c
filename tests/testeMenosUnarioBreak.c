// ===========================================================
// Teste: menos unario + break
// Programa VALIDO (nenhum erro semantico esperado).
//
// Saida esperada:
//   -5 -3.500000 5 7
//   saiu com i = 3
//   a=0 j=1
//   a=1 j=1
//   a=2 j=1
// ===========================================================

// --- menos unario ---
int x = -5;
float y = -3.5;
int z = -x;
int w = - -7;
printf("%d %f %d %d\n", x, y, z, w);

// --- break em laco simples ---
int i = 0;
while (i < 10) {
    if (i == 3) {
        break;
    }
    i = i + 1;
}
printf("saiu com i = %d\n", i);

// --- break em laco aninhado: so deve interromper o laco interno ---
int a = 0;
while (a < 3) {
    int j = 0;
    while (j < 3) {
        if (j == 1) {
            break;
        }
        j = j + 1;
    }
    printf("a=%d j=%d\n", a, j);
    a = a + 1;
}