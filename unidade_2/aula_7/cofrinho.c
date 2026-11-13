#include <stdbool.h>
#include <stdio.h>

int main(void) {
  int amount_in_cents = 0;
  int value;

  printf("--- Cofrinho Digital ---\n");
  printf("Digite 1 para adicionar R$0,50\n");
  printf("Digite 2 para adicionar R$1,00\n");
  printf("Digite 3 para adicionar R$2,00\n");
  printf("(Ctrl+D para sair):\n");

  printf("Total: R$%.2f\n", amount_in_cents / 100.0);
  printf("Status: \n");
  printf("Escolha: ");
  fflush(stdout);

  while (true) {
    int result = scanf("%d", &value);
    
    if (result == EOF) {
      break;
    }

    if (result != 1) {
      break;
    }

    const char *status = "";

    switch (value) {
      case 1:
        amount_in_cents += 50;
        break;
      case 2: 
        amount_in_cents += 100;
        break;
      case 3:
        amount_in_cents += 200;
        break;
      default:
        status = "Opção inválida.";
        break;
    }

    printf("\033[3A\r");
    printf("\033[2KTotal: R$%.2f\n", amount_in_cents / 100.0);
    printf("\033[2KStatus: %s\n", status);
    printf("\033[2KEscolha: ");
    fflush(stdout);
  }

  printf("\nTotal final: R$%.2f\n", amount_in_cents / 100.0);
}
