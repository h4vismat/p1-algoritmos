#include <stdio.h>
#include <stdlib.h>

double calcularMediaTurma(const double *notasTurma) {
    double soma = 0.0;

    for (int i = 0; i < 10; i++) {
        soma += notasTurma[i];
    }

    return soma / 10.0;
}

double encontrarMaiorNota(const double *notasTurma) {
    double notaMaxima = 0.0;

    for (int i = 0; i < 10; i++) {
        if (notasTurma[i] > notaMaxima) {
            notaMaxima = notasTurma[i];
        }
    }

    return notaMaxima;
}

void exibirResultados(const double *notasTurma) {
    double mediaTurma = calcularMediaTurma(notasTurma);
    double maiorNota = encontrarMaiorNota(notasTurma);

    printf("=== RESULTADOS ===\n\n");
    printf("Média da turma: %0.2f\n", mediaTurma);
    printf("Maior nota: %0.2f\n", maiorNota);
}

int main(void) {
    double notasTurma[10];

    for (int i = 0; i < 10; i++) {
        printf("Digite a nota do aluno %d: ", i+1);
        if (scanf("%lf", &notasTurma[i]) != 1) {
            fprintf(stderr, "Valor inválido.\n");
            return EXIT_FAILURE;
        }
    }

    exibirResultados(notasTurma);
    return EXIT_SUCCESS;
}
