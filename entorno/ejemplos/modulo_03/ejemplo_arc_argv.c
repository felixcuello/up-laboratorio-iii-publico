#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Uso: %s <archivo> <cantidad>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Acá ya podemos usar argv[1] y argv[2] con seguridad */
  printf("Archivo: %s\n", argv[1]);
  printf("Cantidad: %s\n", argv[2]);

  return 0;
}
