#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 10;

void verificaParidade(int i) {
  if (i % 2 == 0) {
    printf("Número é par.\n");
  } else {
    printf("Número é ímpar.\n");
  }
}

void positivoOuNegativo(int i) {
  if (i > 0) {
    printf("Número é positivo.\n");
  } else if (i < 0) {
    printf("Número é negativo.\n");
  } else { 
    printf("Número é zero.\n");
  }
}

void calculaQuadrado(int i) {
  printf("O quadrado do número é %d\n", i*i);
}

int main(void) {
  int i, j;

  printf("---- MENU ----\n");
  printf("1) Verificar paridade.\n");
  printf("2) Verificar positividade.\n");
  printf("3) Calcular quadrado.\n");
  printf("Escolha uma ferramenta: ");

  scanf("%d", &i);
  if (i < 1 || i > 3) {
    printf("\nSeleção inválida.\n");
    return 1;
  }

  printf("Digite o valor a ser usado:");
  scanf("%d", &j);

  switch (i) {
    case 1:
      verificaParidade(j);
      break;
    case 2:
      positivoOuNegativo(j);
      break;
    case 3:
      calculaQuadrado(j);
      break;
  }

  return 0;
}
