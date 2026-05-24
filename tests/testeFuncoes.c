// ============================================================
// TESTES - Suporte a Funcoes
// ============================================================

// ── 1. Funcao void (sem retorno) ──────────────────────────
void saudar() {
    printf("Ola! Testando funcoes...\n");
}

// ── 2. Funcao int simples ─────────────────────────────────
int soma(int a, int b) {
    return a + b;
}

// ── 3. Funcao float ───────────────────────────────────────
float media(float x, float y) {
    return (x + y) / 2.0;
}

// ── 4. Funcao bool ────────────────────────────────────────
bool ehPar(int n) {
    return n % 2 == 0;
}

// ── 5. Recursao: fatorial ─────────────────────────────────
int fatorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * fatorial(n - 1);
}

// ── 6. Recursao: fibonacci ────────────────────────────────
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// ── 7. Funcao com if/else interno ────────────────────────
int maximo(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

// ── 8. Funcao com loop interno ───────────────────────────
int somaAte(int n) {
    int total = 0;
    int i = 1;
    while (i <= n) {
        total += i;
        i++;
    }
    return total;
}

// ── 9. Funcao chamando outra funcao ──────────────────────
int somaQuadrados(int a, int b) {
    return soma(a * a, b * b);
}

// ── 10. Funcao char ───────────────────────────────────────
char primeiraLetra(char c) {
    return c;
}

// ============================================================
// EXECUCAO DOS TESTES
// ============================================================

printf("=== TESTE 1: void ===\n");
saudar();

printf("=== TESTE 2: int simples ===\n");
int r1 = soma(3, 4);
printf("soma(3, 4) = %d\n", r1);

printf("=== TESTE 3: float ===\n");
float r2 = media(4.0, 6.0);
printf("media(4.0, 6.0) = %g\n", r2);

printf("=== TESTE 4: bool ===\n");
bool r3 = ehPar(8);
bool r4 = ehPar(7);
printf("ehPar(8) = %d\n", r3);
printf("ehPar(7) = %d\n", r4);

printf("=== TESTE 5: recursao fatorial ===\n");
printf("fatorial(0) = %d\n", fatorial(0));
printf("fatorial(1) = %d\n", fatorial(1));
printf("fatorial(5) = %d\n", fatorial(5));
printf("fatorial(10) = %d\n", fatorial(10));

printf("=== TESTE 6: recursao fibonacci ===\n");
printf("fibonacci(0) = %d\n", fibonacci(0));
printf("fibonacci(1) = %d\n", fibonacci(1));
printf("fibonacci(7) = %d\n", fibonacci(7));
printf("fibonacci(10) = %d\n", fibonacci(10));

printf("=== TESTE 7: if/else interno ===\n");
printf("maximo(10, 3) = %d\n", maximo(10, 3));
printf("maximo(2, 9) = %d\n", maximo(2, 9));
printf("maximo(5, 5) = %d\n", maximo(5, 5));

printf("=== TESTE 8: loop interno ===\n");
printf("somaAte(5) = %d\n", somaAte(5));
printf("somaAte(10) = %d\n", somaAte(10));

printf("=== TESTE 9: funcao chamando funcao ===\n");
printf("somaQuadrados(3, 4) = %d\n", somaQuadrados(3, 4));

printf("=== TESTE 10: char ===\n");
char r5 = primeiraLetra('Z');
printf("primeiraLetra('Z') = %c\n", r5);

printf("=== TESTE 11: chamada inline em expressao ===\n");
int r6 = soma(1, 2) + soma(3, 4);
printf("soma(1,2) + soma(3,4) = %d\n", r6);

printf("=== TODOS OS TESTES CONCLUIDOS ===\n");