#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool isPrime(int x) {
    if (x < 2) return false;
    if (x == 2) return true;

    for (int i = 3; i <= x / i; i += 2) {
        if (x % i == 0) return false;
    }

    return true;
}

int main(void) {
    int x;

    printf("Digite o número para validar se é primo: ");
    if (scanf("%d", &x) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    if (isPrime(x)) {
        printf("O número é primo.\n");
    } else {
        printf("O número não é primo.\n");
    }

    return 0;
}
