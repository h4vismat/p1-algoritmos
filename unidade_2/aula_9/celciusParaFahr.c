#include <stdio.h>
#include <stdlib.h>

// exercício direto The C Programming Language XD, deu nostalgia

double celciusParaFahrenheit(double x) {
    return (9.0 * x) / 5 + 32;
}

int main() {
    double celcius;

    printf("Digite o valor em Celcius: ");
    if (scanf("%lf", &celcius) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    double fahr = celciusParaFahrenheit(celcius);

    printf("O valor em Fahrenheit é %0.2f", fahr);

    return 0;
}
