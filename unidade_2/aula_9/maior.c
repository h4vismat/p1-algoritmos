#include <stdlib.h>
#include <stdio.h>

int maiorNumero(int x, int y) {
    if (x > y)
        return x;
    else
     return y;
}

int main() {
    int x, y;

    printf("Digite o primeiro valor: ");
    if (scanf("%d", &x) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    printf("Digite o segundo valor: ");
    if (scanf("%d", &y) != 2) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    printf("O maior valor é %d", maiorNumero(x, y));

    return 0;
}
