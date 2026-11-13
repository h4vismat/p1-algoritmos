#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Uso: %s digito1 digito2 digito3", argv[0]);
    return 1;
  }

  int digitos[3];
  
  for (int i = 0; i < 3; i++) {
    int valor = atoi(argv[i + 1]);
    digitos[i] = valor;
  }

  int menor = INT_MAX;
  int maior = INT_MIN;
  int intermediario = 0;

  for (int i = 0; i < 3; i++) {
    if (digitos[i] < menor) {
      menor = digitos[i];
    }

    if (digitos[i] > maior) {
      maior = digitos[i];
    }
  }

  for (int i = 0; i < 3; i++) {
    if (digitos[i] > menor && digitos[i] < maior) {
      intermediario = digitos[i];
    }
  }

  printf("------ Resultado ------\n");
  printf("Menor valor: %d\t Maior valor: %d\t Intemediário: %d\n", menor, maior, intermediario);

  printf("Repetição: ");
  if (digitos[0] == digitos[1] || digitos[0] == digitos[2] || digitos[1] == digitos[2]) {
    printf("Existem valores repetidos.\n");
  } else {
    printf("Não existem valores repetidos.\n");
  }

  printf("Igualdade: ");
  if (digitos[0] == digitos[1] && digitos[1] == digitos[2]) {
    printf("Os valores são todos iguais.\n");
  } else {
    printf("Os valores são diferentes.\n");
  }

  printf("Ordem: ");
  if (digitos[0] < digitos[1] && digitos[1] < digitos[2]) {
    printf("Os valores estão em ordem crescente.\n");
  } else if (digitos[2] < digitos[1] && digitos[1] < digitos[0]) {
    printf("Os valores estão em ordem decrescente.\n");
  } else {
    printf("Os valores estão desordenados.\n");
  }
}
