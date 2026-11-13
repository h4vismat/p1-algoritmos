#include <stdio.h>
#include <stdlib.h>

int main(void) {
  float altura;
  float peso;

  printf("Insira a altura: \t");
  scanf("%f", &altura);

  printf("\nInsira o peso: \t");
  scanf("%f", &peso);

  printf("IMC: %.2f", peso / (altura * altura));

  return 0;
}
