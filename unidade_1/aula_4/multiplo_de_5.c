#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s valor", argv[0]);
    return 1;
  }

  if (atoi(argv[1]) % 5 == 0) {
    printf("O valor é múltiplo de 5.");
  } else {
    printf("O valor não é múltiplo de 5.");
  }

  return 0;
}
