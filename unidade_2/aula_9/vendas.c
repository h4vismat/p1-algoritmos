#include <stdio.h>
#include <stdlib.h>

#define N 5

double calcularTotal(double *arr) {
    double total = 0.0;
    for (int i = 0; i < N; i++) {
        total += arr[i];
    }

    return total;
}

double calcularMedia(double total, int qtdDias) {
    return total / (double)qtdDias;
}

int contarVendasAcimaDaMedia(const double vendas[], const double media) {
    int diasAcima = 0;
    for (size_t i = 0; i < N; i++) {
        if (vendas[i] > media) diasAcima++;
    }

    return diasAcima;
}

int main(void) {
    double vendasPorDia[N];

    for (int i = 0; i < N; i++) {
        double valor;

        printf("Digite o valor de vendas do dia %d: ", i+1);
        if (scanf("%lf", &valor) != 1) {
            fprintf(stderr, "Valor inválido.\n");
            return EXIT_FAILURE;
        }

        vendasPorDia[i] = valor;
    }

    double total = calcularTotal(vendasPorDia);
    double media = calcularMedia(total, N);
    int diasAcimaDaMedia = contarVendasAcimaDaMedia(vendasPorDia, media);

    printf("=== RESULTADOS ===\n\n");
    printf("Total: %0.2f\n", total);
    printf("Media: %0.2f\n", media);
    printf("Dias acima da media: %d\n", diasAcimaDaMedia);

    return EXIT_SUCCESS;
}
