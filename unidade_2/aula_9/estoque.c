#define N 8

int calcularTotalEstoque(const int produtos[]) {
    int total = 0;

    for (int i = 0; i < N; i++) {
        int qtd = produtos[i];
        total += qtd;
    }

    return total;
}

int contarEstoqueBaixo(const int produtos[]) {
    int qtd = 0;

    for (int i = 0; i < N; i++) {
        if (produtos[i] < 5) qtd++;
    }

    return qtd;
}

int buscarProduto(const int produtos[], int codProduto) {
    if (codProduto < 1 || codProduto > N) return -1;

    return produtos[codProduto-1];
}

int main(void) {
    // Joanacelle, não entendi qual era a proposta da main dessa daqui. Seria um menu?
    return 0;
}
