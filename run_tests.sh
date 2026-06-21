#!/usr/bin/env bash
# run_tests.sh - executa a suite de testes de regressao do interpretador C
#
# Uso:
#   ./run_tests.sh [diretorio_de_testes] [caminho_do_binario]
#
# Estrutura esperada do diretorio de testes (padrao: ./testes):
#   testes/
#     caso1.c
#     caso1.expected   <- saida esperada (stdout) ao executar caso1.c
#     caso2.c
#     caso2.expected
#     ...
#
# Aceita tanto .c quanto .C (case-sensitive no Linux, mas alguns arquivos
# do projeto usam .C maiusculo).
#
# Qualquer .c/.C sem .expected correspondente eh ignorado (SKIP), nao falha.

set -uo pipefail

TESTS_DIR="${1:-tests}"
BIN="${2:-./programa}"

PASS=0
FAIL=0
SKIP=0
FAILED_TESTS=()

if [ -t 1 ]; then
    GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[0;33m'; NC='\033[0m'
else
    GREEN=''; RED=''; YELLOW=''; NC=''
fi

if [ ! -x "$BIN" ]; then
    echo -e "${RED}Erro:${NC} binario '$BIN' nao encontrado ou sem permissao de execucao."
    echo "Compile o interpretador antes de rodar os testes (ex.: make)."
    exit 1
fi

if [ ! -d "$TESTS_DIR" ]; then
    echo -e "${RED}Erro:${NC} diretorio de testes '$TESTS_DIR' nao existe."
    exit 1
fi

shopt -s nullglob
ARQUIVOS=("$TESTS_DIR"/*.c "$TESTS_DIR"/*.C)
NOMES_VISTOS=()

for src in "${ARQUIVOS[@]}"; do
    nome=$(basename "$src")
    nome="${nome%.*}"

    # evita processar o mesmo caso duas vezes se existir .c e .C juntos
    if printf '%s\n' "${NOMES_VISTOS[@]}" | grep -qx "$nome"; then
        continue
    fi
    NOMES_VISTOS+=("$nome")

    esperado="$TESTS_DIR/$nome.expected"

    if [ ! -f "$esperado" ]; then
        echo -e "${YELLOW}[SKIP]${NC} $nome (sem arquivo .expected)"
        SKIP=$((SKIP+1))
        continue
    fi

    saida_real=$("$BIN" "$src" 2>&1)
    saida_esperada=$(cat "$esperado")

    if [ "$saida_real" == "$saida_esperada" ]; then
        echo -e "${GREEN}[OK]  ${NC} $nome"
        PASS=$((PASS+1))
    else
        echo -e "${RED}[FAIL]${NC} $nome"
        FAIL=$((FAIL+1))
        FAILED_TESTS+=("$nome")
        diff <(echo "$saida_esperada") <(echo "$saida_real") | sed 's/^/      /'
    fi
done

echo ""
echo "----------------------------------------"
echo "Resultado: $PASS ok, $FAIL falha(s), $SKIP ignorado(s)"

if [ "$FAIL" -gt 0 ]; then
    echo ""
    echo "Testes com falha:"
    for t in "${FAILED_TESTS[@]}"; do
        echo "  - $t"
    done
    exit 1
fi

exit 0