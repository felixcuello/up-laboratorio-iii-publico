#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  FILE *fp;
  long timestamp;
  char metrica[64];
  float valor;
  int leidos;

  /* Etapa 1: escribir mediciones simuladas en metrics.log */
  fp = fopen("metrics.log", "w");
  if (fp == NULL) {
    perror("Error al abrir metrics.log para escritura");
    exit(EXIT_FAILURE);
  }

  fprintf(fp, "%ld %s %.2f\n", 1714665600L, "cpu.usage", 87.45);
  fprintf(fp, "%ld %s %.2f\n", 1714665601L, "memory.usage", 62.10);
  fprintf(fp, "%ld %s %.2f\n", 1714665602L, "disk.io", 120.33);
  fprintf(fp, "%ld %s %.2f\n", 1714665603L, "cpu.usage", 91.02);
  fprintf(fp, "%ld %s %.2f\n", 1714665604L, "network.in", 45.78);

  if (fclose(fp) == EOF) {
    perror("Error al cerrar metrics.log despues de escribir");
    exit(EXIT_FAILURE);
  }

  /* Etapa 2: releer el archivo y mostrar las mediciones */
  fp = fopen("metrics.log", "r");
  if (fp == NULL) {
    perror("Error al abrir metrics.log para lectura");
    exit(EXIT_FAILURE);
  }

  printf("=== Mediciones registradas ===\n");
  leidos = fscanf(fp, "%ld %s %f", &timestamp, metrica, &valor);

  while (leidos == 3) {
    printf("[%ld] %-15s -> %.2f\n", timestamp, metrica, valor);
    leidos = fscanf(fp, "%ld %s %f", &timestamp, metrica, &valor);
  }

  if (!feof(fp)) {
    fprintf(stderr, "Error: el archivo tiene un formato invalido\n");
    exit(EXIT_FAILURE);
  }

  if (fclose(fp) == EOF) {
    perror("Error al cerrar metrics.log despues de leer");
    exit(EXIT_FAILURE);
  }

  return 0;
}
