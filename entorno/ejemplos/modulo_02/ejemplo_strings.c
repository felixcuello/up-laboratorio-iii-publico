#include <stdio.h>
#include <string.h>

int main() {
  char saludo[] = "Hola"; /* definicion de un string sin especificar el tamaño del arreglo */
  char saludo2[10] = "Hola"; /* definicion de un string especificando el tamaño del arreglo */

  printf("%s\n", saludo);

  /* sizeof devuelve el tamaño en bytes de un tipo de dato o variable */
  printf("El tamaño del primer saludo es: %lu\n", sizeof(saludo));
  printf("El tamaño del segundo saludo es: %lu\n", sizeof(saludo2));

  /* strlen devuelve la longitud de un string hasta el caracter nulo '\0' */
  printf("La longitud del primer saludo es: %lu\n", strlen(saludo));

  return 0;
}
