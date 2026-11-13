#include <stdio.h>

void tabuada(int x) {
    for (int i = 1; i <= 10; i++) {
        printf("%d x %d = %d\n", x, i, x*i);
    }
}

int main(void) {
    int x;

    printf("Digite o valor para ver a tabuada: ");
    if (scanf("%d", &x) != 1) {
        fprintf(stderr, "Valor inválido.\n");
    }

    tabuada(x);

    return 0;
}
