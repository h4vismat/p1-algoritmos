#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Uso: %s digito1 digito2", argv[0]);
  }

  int digitos[2];

  for (int i = 0; i < 2; i++) {
    digitos[i] = atoi(argv[i+1]);
  }

  int resultado = digitos[0] - digitos[1];
  printf("Resultado: %d", resultado);

  return 0;
}
