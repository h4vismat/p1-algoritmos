#include <stdio.h>
#include <stdlib.h>

int add(int x, int y) {
    return x + y;
}

int subtract(int x, int y) {
    return x - y;
}

int multiply(int x, int y) {
    return x * y;
}

int divide(int x, int y) {
    return x / y;
}

int scanValues(int *x, int *y) {
    printf("Digite o primeiro termo: ");

    if (scanf("%d", x) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return 0;
    }

    printf("Digite o segundo termo: ");

    if (scanf("%d", y) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return 0;
    }

    return 1;
}

int main(void) {
    int x, y;
    char comando;

    printf("=== CALCULADORA ===\n\n");
    printf("Selecione a operação:\n");
    printf("1) Adição.\n");
    printf("2) Subtração.\n");
    printf("3) Multiplicação.\n");
    printf("4) Divisão.\n");
    printf("\n0) Sair.\n");

    printf("\nDigite a operação: ");

    if (scanf("%c", &comando) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    switch (comando) {
        case '1':
            if (scanValues(&x, &y) != 1) {
                return EXIT_FAILURE;
            }

            printf("O valor é %d\n", add(x, y));
            break;

        case '2':
            if (scanValues(&x, &y) != 1) {
                return EXIT_FAILURE;
            }

            printf("O valor é %d\n", subtract(x, y));
            break;

        case '3':
            if (scanValues(&x, &y) != 1) {
                return EXIT_FAILURE;
            }

            printf("O valor é %d\n", multiply(x, y));
            break;

        case '4':
            if (scanValues(&x, &y) != 1) {
                return EXIT_FAILURE;
            }

            if (y == 0) {
                printf("Não é possível dividir por 0.\n");
                return EXIT_FAILURE;
            }

            printf("O valor é %d\n", divide(x, y));
            break;

        case '0':
            return EXIT_SUCCESS;

        default:
            printf("Opção inválida.\n");
            break;
    }
}
