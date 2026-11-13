#include <stdio.h>
#include <stdlib.h>

double calcularMedia(double a, double b, double c) {
    return (a + b + c) / 3;
}

void verificarSituacao(double media) {
    if (media > 7.0) {
        printf("Aluno aprovado.\n");
    } else {
        printf("Aluno reprovado.\n");
    }
}

int main(void) {
    double nota1, nota2, nota3;

    printf("Digite a primeira nota: ");
    if (scanf("%lf", &nota1) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    printf("Digite a segunda nota: ");
    if (scanf("%lf", &nota2) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    printf("Digite a terceira nota: ");
    if (scanf("%lf", &nota3) != 1) {
        fprintf(stderr, "Valor inválido.\n");
        return EXIT_FAILURE;
    }

    double media = calcularMedia(nota1, nota2, nota3);
    verificarSituacao(media);

    return EXIT_SUCCESS;
}
