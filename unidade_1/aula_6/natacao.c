#include <stdio.h>
#include <stdlib.h>

void printCategoria(int idade, char *nomeCategoria) {
  printf("Nadador de idade %d é da categoria %s", idade, nomeCategoria);
}

int main(void) {
  int idade;

  printf("Digite a idade do nadador para avaliação.\n");
  scanf("%d", &idade);

  if (idade >= 5 && idade <= 7) {
    char categoria[] = "Infantil A";
    printCategoria(idade, categoria);
  } else if (idade >= 8 && idade <= 10) {
    char categoria[] = "Infantil B";
    printCategoria(idade, categoria);
  } else if (idade >= 11 && idade <= 13) {
    char categoria[] = "Juvenil A";
    printCategoria(idade, categoria);
  } else if (idade >= 14 && idade <= 17) {
    char categoria[] = "Juvenil C";
    printCategoria(idade, categoria);
  } else if (idade > 17) {
    char categoria[] = "Sênior";
    printCategoria(idade, categoria);
  } else {
    printf("Categoria inválida");
  }

  return 0;
}
