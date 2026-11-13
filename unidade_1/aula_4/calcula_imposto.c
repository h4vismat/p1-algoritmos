#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s valor", argv[0]);
    return 1;
  }

  printf("Valor total com impostos: %.2f", strtof(argv[1], NULL) * 1.08);
  return 0;
}
