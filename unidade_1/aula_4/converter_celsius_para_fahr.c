#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s valor", argv[0]);
    return 1;
  }

  printf("Valor em fahrenheit: %.2f", strtof(argv[1], NULL) * 9.0 / 5.0 + 32.0);
  return 0;
}
