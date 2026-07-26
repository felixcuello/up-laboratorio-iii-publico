# Modulos

En este directorio encontrarás algunos de los archivos de código fuente que forman parte de los módulos de la materia.
Los dejamos acá para que te sea más fácil usarlos directamente desde el entorno de desarrollo sin necesidad de tener que
copiar y pegarlos desde la plataforma de la materia.

**Forma de compilar:** Esta es la forma de compilar un programa con sus módulos (si hubiera uno o más). Tené en cuenta
que no incluye bibliotecas externas, por lo que si tu programa las necesita, deberás agregarlas a la línea de
compilación (ej: `-lpthread` para usar hilos).

```sh
gcc -std=c89 -pedantic -Wall -Werror -o programa programa.c modulo1.c modulo2.c
```
