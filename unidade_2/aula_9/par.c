#include <stdio.h>
#include <stdlib.h>

int ehPar(int x) {
    if (x % 2 == 0)
        return 1;
    else
        return 0;
}

int main() {
    int x;

    printf("Digite o valor: ");
    if (scanf("%d", &x) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    if (ehPar(x) == 1) {
        printf("O valor é par.\n");
    } else {
        printf("O valor é ímpar.\n");
    }

    return 0;
}
