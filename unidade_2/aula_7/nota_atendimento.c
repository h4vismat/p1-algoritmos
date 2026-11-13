#include <math.h>
#include <stdio.h> 
#include <stdlib.h>

#define NUM_CLIENTES 10
#define LIMITE_ALERTA 7.0

typedef struct {
  double media;
  double mediana;
  double desvio_padrao;
  int minimo;
  int maximo;
  int insatisfeitos;
} Estatisticas;

int ler_nota(int cliente) {
  int nota;

  while (1) {
    printf("Nota do cliente %d: ", cliente);

    if (scanf("%d", &nota) != 1) {
      fprintf(stderr, "Entrada inválida.\n");
      exit(EXIT_FAILURE);
    }

    if (nota > 0 && nota <= 10) {
      return nota;
    }

    printf("A nota deve estar entre 0 e 10.\n");
  }
}

int comparar(const void *a, const void *b) {
  int x = *(const int *)a;
  int y = *(const int *)b;

  return (x > y) - (x < y);
}

Estatisticas analisar(const int notas[], int quantidade) {
  Estatisticas stats = {0};

  int soma = 0;
  stats.minimo = notas[0];
  stats.maximo = notas[0];

  for (int i = 0; i < quantidade; i++) {
    int nota = notas[i];
    soma += nota;

    if (nota < stats.minimo)
      stats.minimo = nota;

    if (nota > stats.maximo)
      stats.maximo = nota;

    if (nota < 7)
      stats.insatisfeitos++;
  }

  stats.media = (double)soma / quantidade;

  double variancia = 0;
  
  for (int i = 0; i < quantidade; i++) {
    double diferenca = notas[i] - stats.media;
    variancia += diferenca * diferenca;
  }

  stats.desvio_padrao = sqrt(variancia / quantidade);

  int ordenadas[NUM_CLIENTES];

  for (int i = 0; i < quantidade; i++) {
    ordenadas[i] = notas[i];
  }

  qsort(ordenadas, quantidade, sizeof(int), comparar);

  if (quantidade % 2 == 0) {
    stats.mediana = (ordenadas[quantidade / 2 - 1] + ordenadas[quantidade / 2]) / 2.0;
  } else {
    stats.mediana = ordenadas[quantidade / 2];
  }

  return stats;
}

void exibir_histograma(const int notas[], int quantidade) {
  int frequencias[11] = {0};

  for (int i = 0; i < quantidade; i++) {
    frequencias[notas[i]]++;
  }

  printf("\nDistribuição das notas: \n");
  for (int nota = 0; nota <= 10; nota ++) {
    printf("%2d | ", nota);

    for (int j = 0; j < frequencias[nota]; j++) {
      printf("#");
    }

    printf(" (%d)\n", frequencias[nota]);
  }
}

int main(void) {
  int notas[NUM_CLIENTES];

  printf("--- Pesquisa de satisfação ---\n\n");

  for (int i = 0; i < NUM_CLIENTES; i++) {
    notas[i] = ler_nota(i + 1);
  }

  Estatisticas stats = analisar(notas, NUM_CLIENTES);

  printf("\n--- RELATÓRIO ---\n");
  printf("Média:          %.2f\n", stats.media);
  printf("Mediana:        %.2f\n", stats.mediana);
  printf("Desvio padrão:  %.2f\n", stats.desvio_padrao);
  printf("Menor nota:     %d\n", stats.minimo);
  printf("Maior nota:     %d\n", stats.maximo);
  printf("Insatisfeitos:  %d\n", stats.insatisfeitos);

  if (stats.media < LIMITE_ALERTA) {
    printf("\nAlerta: satisfação abaixo do esperado.\n");
  } else {
    printf("\nSatisfação dentro do esperado.\n");
  }

  exibir_histograma(notas, NUM_CLIENTES);

  return 0;
}
