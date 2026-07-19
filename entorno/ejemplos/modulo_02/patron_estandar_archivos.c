#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  FILE *fp;
  fp = fopen("metrics.log", "r");

  if (fp == NULL) {
    perror("Error al abrir metrics.log");
    exit(EXIT_FAILURE);
  }

  /* Acá ya podemos operar con fp con seguridad */
  return 0;
}
