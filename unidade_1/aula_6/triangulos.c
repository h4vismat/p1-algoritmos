#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Uso: %s lado1 lado2 lado3", argv[0]);
    return 1;
  }

  long long lados[3];

  for (int i = 0; i < 3; i++) {
    char *fim;
    long valor = strtol(argv[i + 1], &fim, 10);

    lados[i] = valor;
  }

  // Ordena os lados.
  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (lados[i] > lados[j]) {
        long long aux = lados[i];
        lados[i] = lados[j];
        lados[j] = aux;
      }
    }
  }

  long long a = lados[0];
  long long b = lados[1];
  long long c = lados[2];

  if (a <= 0 || a + b <= c) {
    printf("Os valores não formam um triângulo.\n");
    return 0;
  }

  if (a == c) {
    printf("Quanto aos lados: equilátero.\n");
  } else if (a == b || a == c) {
    printf("Quanto aos lados: isósceles.\n");
  } else {
    printf("Quanto aos lados: escaleno.\n");
  }

  long long soma = a * a + b * b;
  long long maior = c * c;

  if (maior == soma) {
    printf("Quanto aos ângulos: retângulo\n");
  } else if (maior < soma) {
    printf("Quanto aos ângulos: acutângulo.\n");
  } else {
    printf("Quanto aos ângulos: obtusângulo.\n");
  }
}
