# Trabajo Práctico Especial - SO 2023

## Tabla de contenidos

- [Requerimientos](#requerimientos)
- [GitFlow](#gitflow)
  - [Inicializando un gitflow repo](#inicializando-un-gitflow-repo)
  - [Manejando ramas](#manejando-ramas)
- [Compilando](#compilando)
- [Problemas encontrados](#problemas-encontrados)
  - [Shared Memory](#shared-memory)
  - [Punteros en memoria compartida](#punteros-en-memoria-compartida)
  - [Problema 3](#problema-3)
- [Decisiones tomadas](#decisiones-tomadas)
  - [TADs](#tads)
  - [Decision 2](#decision-2)
  - [Decision 3](#decision-3)
- [Limitaciones](#limitaciones)
- [Codigo reutilizado](#codigo-reutilizado)
- [Diagrama](#diagrama)

## Requerimientos

Para el trabajo en cuestión se estarán utilizando los siguientes componentes:

- [gitflow plugin](https://github.com/nvie/gitflow)
- docker

El plugin de gitflow (explicado en [GitFlow](#gitflow)) permite manejar los branch de `git` de una forma más sencilla que la habitual, siguiendo la filosofía del modelado de ramas de [gitflow](https://nvie.com/posts/a-successful-git-branching-model/).

## GitFlow

Para instalar `gitflow` simplemente se necesita usar el _package manager_ de la distribución en la que trabajes:

**Para Ubuntu o Debian**

```bash
$ apt install git-flow
```

**Para Arch**

```bash
$ yay -S gitflow-avh
```

`gitflow` se basa en la creación de ramas promoviendo una separación entre **desarrollo** (_develop_) y **lanzamientos** (_releases_), utilizando ramificaciones de _features_ cortos que se mergean a la rama de desarrollo.

Para simplificar el manejo de dichas ramas, dicho plugin para git crea comandos que resumen una serie de pasos para asegurarse la mejor implementación de dicho modelado.

### Inicializando un gitflow repo

Para inicializar el modelado de gitflow en el repositorio actual debemos correr alguno de los siguientes comandos:

```bash
$ git flow init
$ git flow init -d  # toma los valores por default
```

### Manejando ramas

Ahora para el manejo de todas las ramas y del repo en general, transcribo del [README de gitflow](https://github.com/nvie/gitflow/blob/develop/README.mdown), el manual de como manejar las ramas.

- To list/start/finish feature branches, use:

  ```bash
  git flow feature
  git flow feature start <name> [<base>]
  git flow feature finish <name>
  ```

  For feature branches, the `<base>` arg must be a commit on `develop`.

- To push/pull a feature branch to the remote repository, use:

  ```bash
  git flow feature publish <name>
  git flow feature pull <remote> <name>
  ```

- To list/start/finish release branches, use:

  ```bash
  git flow release
  git flow release start <release> [<base>]
  git flow release finish <release>
  ```

  For release branches, the `<base>` arg must be a commit on `develop`.

- To list/start/finish hotfix branches, use:

  ```bash
  git flow hotfix
  git flow hotfix start <release> [<base>]
  git flow hotfix finish <release>
  ```

  For hotfix branches, the `<base>` arg must be a commit on `master`.

- To list/start support branches, use:
  ```bash
  git flow support
  git flow support start <release> <base>
  ```
  For support branches, the `<base>` arg must be a commit on `master`.

## Compilando

Para compilar el proyecto es tan sencillo como correr el script `compile.sh` que se encarga del manejo de la imágen de docker para compilar el proyecto igual para cualquiera que haya clonado el presente repositorio.

```bash
$ ./compile.sh
```

## Problemas encontrados

### Shared memory

Al estar investigando acerca del tema de "_shared memory_", pudimos entender varios aspectos del tema. Implementarlo en el proyecto no tuvo mucho problema mas que utilizamos las syscalls que quedaron obsoletas con el paso del tiempo, como pueden ser "_shmget_", "_shmat_" o "_shmdt_". Por esta razon, tuvimos que cambiar las funciones creadas y usar syscalls tales como "_shmopen_" y "_mmap_", que por lo que entendimos son el estandar de hoy en dia.

El verdadero problema, vino al implementar los semaforos. Cuando incluimos los semaforos en el codigo descubrimos un gran problema al ejecutar el vista, este programa nunca termina. Como el proceso vista solo recibe la informacion necesaria para conectarse a la "_shared memory_" por entrada estandar no tenemos manera de decirle a el proceso vista cuando ya no hay mas archivos para leer. Al ejecutar el programa, podiamos apreciar como habia un buen funcionamiento de la shared memory entre el proceso aplicaion y el proceso vista, pero cuando terminaba de imprimir los archivos, el proceso aplicacion terminaba su ejecuccion y el proceso vista se queda esperando en el semaforo por el siguiente elemento.

Nuestra primera solucion a esto fue investigar la API de semaforos POSIX. Investigando, encontramos "sem_timedwait" la cual hace la espera como el semaforo que habiamos incluido anteriormente y si en un rango de tiempo (programado) el semaforo no cambia de valor, inmediatamente se termina este proceso, lo cual permite que el proceso visto pueda terminar su ejecucion. Esta solucion no es la correcta ya que no funciona en todos los casos. Si da la casualidad de que todos los procesos escalvos esten trabajando en un archivo muy grande, estos pueden llegar a tardar mucho tiempo lo cual llevaria a el proceso vista a terminar antes de lo deberia

La solucion final que implementamos feu incluir un "_end of file_" en el TAD del semaforo. Por lo tanto el proceso aplicacion modificara este "_end of file_" cuando alla terminado de trabajar con todos los archivos disponibles, y el proceso vista chequea en cada iteracion antes de esperar en el semaforo si este "_end of file_" fue modificado o no.

### Punteros en memoria compartida

Al declarar el TAD de la "_shared memory_" teniamos un error que no nos perimitia destruir o desincronizar el bloque de memoria creado anteriormente. Tras mucho "_debugging_" descubrimos que le problema yacia dentro de la declaracion de nuestra variable "_path_" dentro del TAD, la cual anteriormente era declarada como puntero. Como era declarado como puntero, al crear la memoria compartida se guardaba el valor "_path_" recibido por paramtero de esta manera. Esto es un error ya que se guardaba este parametro de manera que apuntara a una direccion de memoria de un proceso especifico. Al quere borrar o cerrar la coneccion con la memoria compartido, lanzaba un error, ya que el puntero no correspondia. La solucion fue declarar esta variable como un array estatico y de esta manera solucionar los errores y poder desaclopar la coneccion a la memoria compartido y luego borrar el bloque.

### Problema 3

Más en el comienzo del desarrollo, tuvimos un problema con el uso de 'select', el programa quedaba colgado en el select. Resulta que el slave no estaba escribiendo sus resultados correctamente en el pipe, printf no lograba escribir en el fd 1 (que estaba mapeado al pipe). Finalmente logramos solucionarlo mediante setvbuffer().

## Decisiones tomadas

### TADs

Una de la decisiones mas importantes que tomamos a principio del proyecto fue trabajar con dos TADs (Tipos Abstractos de Datos). Creamos dos TADs, uno para guardar la informacion de los esclavos y otro para la infromacion de la memoria compartida. Al crear las librerias, tanto "_slave manager_" como "_shm lib_", se facilita mucho la modularizacion del codigo y, al crear los "_struct_" que agrupen tantas variables, el codigo queda mucho mas limpio y agradable a la vista. Tambien se facilita el pasaje y recibo de parametros en diversas funciones debido a que se puede enviar y recibir la estructura como parametro y no estar pasando parametros individualmente.

### Decision 2

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque ac erat nunc. Nulla arcu tellus, pharetra ut tempor in, facilisis id tellus. Aliquam pulvinar dui sit amet mauris lacinia, dictum vehicula lectus fringilla. Donec id odio euismod, blandit ex quis, efficitur nunc. Aliquam non justo hendrerit, lobortis justo nec, ornare eros. Fusce dignissim, dolor sed fringilla egestas, nibh purus efficitur magna, in sollicitudin libero elit lobortis enim. Nam fermentum venenatis diam, vel congue odio cursus ut. Duis sapien ex, dictum sit amet sollicitudin eu, dictum non nunc. Praesent sapien enim, venenatis eget lacus quis, imperdiet euismod nisl.

Vivamus in mauris faucibus, cursus ex at, porttitor libero. Integer accumsan enim at orci pharetra luctus. Nam porttitor arcu quis sapien dictum cursus. Maecenas sit amet suscipit metus. Sed nunc nunc, venenatis quis dignissim ornare, dapibus ac elit. Etiam a mi odio. Mauris sit amet consectetur neque, eu mattis justo.

### Decision 3

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque ac erat nunc. Nulla arcu tellus, pharetra ut tempor in, facilisis id tellus. Aliquam pulvinar dui sit amet mauris lacinia, dictum vehicula lectus fringilla. Donec id odio euismod, blandit ex quis, efficitur nunc. Aliquam non justo hendrerit, lobortis justo nec, ornare eros. Fusce dignissim, dolor sed fringilla egestas, nibh purus efficitur magna, in sollicitudin libero elit lobortis enim. Nam fermentum venenatis diam, vel congue odio cursus ut. Duis sapien ex, dictum sit amet sollicitudin eu, dictum non nunc. Praesent sapien enim, venenatis eget lacus quis, imperdiet euismod nisl.

Vivamus in mauris faucibus, cursus ex at, porttitor libero. Integer accumsan enim at orci pharetra luctus. Nam porttitor arcu quis sapien dictum cursus. Maecenas sit amet suscipit metus. Sed nunc nunc, venenatis quis dignissim ornare, dapibus ac elit. Etiam a mi odio. Mauris sit amet consectetur neque, eu mattis justo.

## Limitaciones

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque ac erat nunc. Nulla arcu tellus, pharetra ut tempor in, facilisis id tellus. Aliquam pulvinar dui sit amet mauris lacinia, dictum vehicula lectus fringilla. Donec id odio euismod, blandit ex quis, efficitur nunc. Aliquam non justo hendrerit, lobortis justo nec, ornare eros. Fusce dignissim, dolor sed fringilla egestas, nibh purus efficitur magna, in sollicitudin libero elit lobortis enim. Nam fermentum venenatis diam, vel congue odio cursus ut. Duis sapien ex, dictum sit amet sollicitudin eu, dictum non nunc. Praesent sapien enim, venenatis eget lacus quis, imperdiet euismod nisl.

Vivamus in mauris faucibus, cursus ex at, porttitor libero. Integer accumsan enim at orci pharetra luctus. Nam porttitor arcu quis sapien dictum cursus. Maecenas sit amet suscipit metus. Sed nunc nunc, venenatis quis dignissim ornare, dapibus ac elit. Etiam a mi odio. Mauris sit amet consectetur neque, eu mattis justo.

## Codigo reutilizado

No utilizamos ninguna fuente externa aparte del "_man page_". Basamos partes de nuestro codigo en los ejemplos que proporciona este "_man page_", mas notablemente, para la realizacion de la libreria de "_shared memory_" tomamos mucha inspiracion del codigo de ejemplo que proporicona acerca de "_shmopen_".

## Diagrama
