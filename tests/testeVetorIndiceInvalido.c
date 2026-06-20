// ===========================================================
// Teste: indice fora dos limites (erro em tempo de execucao,
// nao trava o programa, so reporta e segue)
//
// Saida esperada:
//   Erro RT: indice 10 fora dos limites de 'v' (tamanho 3).
// ===========================================================

int v[3];
v[10] = 5;