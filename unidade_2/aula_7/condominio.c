#include <stdio.h>

#define MEDIA 20.0

void mensurar_consumo_individual(float m) {
  if (m > MEDIA) {
    printf("O consumo está acima da média.\n");
  } else {
    printf("O consumo está dentro da média.\n");
  }

}

int main(void) {
  float consumo_total = 0.0;

  for (int i = 0; i < 5; i++) {
    float consumo_individual;
    printf("Digite o consumo do morador %d: ", i+1);
    scanf("%f", &consumo_individual);

    mensurar_consumo_individual(consumo_individual);
    consumo_total += consumo_individual;
  }

  printf("O consumo médio geral foi de %.2f", consumo_total / 5.0);

  return 0;
}
