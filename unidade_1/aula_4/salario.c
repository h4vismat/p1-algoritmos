#include <stdio.h>
#include <stdlib.h>

int main(void) {
  float horas;
  float valorHora;

  printf("Insira a qtd de horas: ");
  scanf("%f", &horas);

  printf("\nInsira o valor da hora: ");
  scanf("%f", &valorHora);

  printf("Salário: %.2f", horas * valorHora);

  return 0;
}
