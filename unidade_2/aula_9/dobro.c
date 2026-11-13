#include <stdlib.h>
#include <stdio.h>

int calcularDobro(int x) {
    return x * 2;
}

int main() {
    int n;

    printf("Digite um número: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    int dobro = calcularDobro(n);
    printf("O dobro é %d", dobro);

    return 0;
}
