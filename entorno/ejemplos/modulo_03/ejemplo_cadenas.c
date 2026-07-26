#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Cuenta las vocales de una cadena */
int contar_vocales(const char *s) {
  int i;
  int cuenta = 0;
  char c;

  for (i = 0; s[i] != '\0'; i++) {
    /*
     * pasamos a minúsculas para no tener que comparar con mayúsculas
     *
     * Una forma de implementar to_lower es restando 32 al valor ASCII de la letra mayúscula, pero esto no es portable y
     * no funciona con caracteres acentuados. Por eso usamos la función tolower(), pero puede ser un buen ejercicio de C
     * para hacer si tenés ganas!
     *
     */
    c = tolower(s[i]);
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
      cuenta++;
    }
  }
  return cuenta;
}

/*
 * Devuelve 1 si la cadena es un palíndromo, 0 si no lo es
 *
 * En esta función utilizamos una técnica algorítmica llamada de "dos punteros" donde se colocan dos índices uno al
 * principio y el otro al final de la cadena y se van moviendo hacia el centro comparando los caracteres (ignorando
 * espacios). Una vez que los punteros se cruzan, si no se ha encontrado ninguna diferencia podemos decir que la cadena
 * ES un palíndromo.
 */
int es_palindromo(const char *s) {
  int i;
  int j;

  i = 0;
  j = strlen(s) - 1;
  while (i < j) {
    /* Saltar espacios desde la izquierda */
    while (i < j && s[i] == ' ') {
      i++;
    }
    /* Saltar espacios desde la derecha */
    while (i < j && s[j] == ' ') {
      j--;
    }
    if (tolower(s[i]) != tolower(s[j])) {
      return 0;
    }
    i++;
    j--;
  }
  return 1;
}

/* Invierte la cadena dada (modifica el string original) */
void invertir_cadena(char *s) {
  int i;
  int j;
  char temp;

  i = 0;
  j = strlen(s) - 1;
  while (i < j) {
    temp = s[i];
    s[i] = s[j];
    s[j] = temp;
    i++;
    j--;
  }
}

/*
 * Cuenta la cantidad de palabras en una cadena (separadas por espacios, tabs o saltos de línea)
 */
int contar_palabras(const char *s) {
  int i;
  int cuenta = 0;
  int dentro_de_palabra = 0;

  for (i = 0; s[i] != '\0'; i++) {
    if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
      dentro_de_palabra = 0;
    } else if (!dentro_de_palabra) {
      dentro_de_palabra = 1;
      cuenta++;
    }
  }
  return cuenta;
}

/* Convierte la cadena a mayúsculas (modifica el string original) */
void a_mayusculas(char *s) {
  int i;

  for (i = 0; s[i] != '\0'; i++) {
    s[i] = toupper(s[i]);
  }
}

int main(void) {
  char texto[] = "anita lava la tina";
  char copia[100];

  strcpy(copia, texto);

  printf("Texto: \"%s\"\n", texto);
  printf("Vocales: %d\n", contar_vocales(texto));
  printf("Palabras: %d\n", contar_palabras(texto));
  printf("Es palindromo: %s\n", es_palindromo(texto) ? "si" : "no");

  a_mayusculas(copia);
  printf("En mayusculas: \"%s\"\n", copia);

  invertir_cadena(copia);
  printf("Invertido: \"%s\"\n", copia);

  return 0;
}

