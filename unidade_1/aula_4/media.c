#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 6) {
    fprintf(stderr, "Uso: %s digito1 digito2 digito3 digito4 digito5", argv[0]);
    return 1;
  }

  int soma = 0;
  for (int i = 0; i < 5; i++) {
    soma = soma + atoi(argv[i+1]);
  }

  printf("Média aritmética: %d", soma/5);
}
