# Modulos

En este directorio encontrarás algunos de los archivos de código fuente que forman parte de los módulos de la materia.
Los dejamos acá para que te sea más fácil usarlos directamente desde el entorno de desarrollo sin necesidad de tener que
copiar y pegarlos desde la plataforma de la materia.

**Forma de compilar:** Como este módulo es de memoria compartida, la forma de compilar será así:

```sh
gcc -D_DEFAULT_SOURCE -std=c89 -pedantic -Wall -Werror -lrt -o productor productor.c
gcc -D_DEFAULT_SOURCE -std=c89 -pedantic -Wall -Werror -lrt -o consumidor consumidor.c
```
