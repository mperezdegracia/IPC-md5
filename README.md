# Trabajo Práctico Especial - SO 2023

## Tabla de contenidos

* [Requerimientos](#requerimientos)
* [GitFlow](#gitflow)
  * [Inicializando un gitflow repo](#inicializando-un-gitflow-repo)
  * [Manejando ramas](#manejando-ramas)
* [Compilando](#compilando)

## Requerimientos

Para el trabajo en cuestión se estarán utilizando los siguientes componentes:

* [gitflow plugin](https://github.com/nvie/gitflow)
* docker

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

* To list/start/finish feature branches, use:
  ```bash
  git flow feature
  git flow feature start <name> [<base>]
  git flow feature finish <name>
  ```
  For feature branches, the `<base>` arg must be a commit on `develop`.

* To push/pull a feature branch to the remote repository, use:
  ```bash
  git flow feature publish <name>
  git flow feature pull <remote> <name>
  ```

* To list/start/finish release branches, use:
  ```bash
  git flow release
  git flow release start <release> [<base>]
  git flow release finish <release>
  ```
  For release branches, the `<base>` arg must be a commit on `develop`.

* To list/start/finish hotfix branches, use:
  ```bash
  git flow hotfix
  git flow hotfix start <release> [<base>]
  git flow hotfix finish <release>
  ```
  For hotfix branches, the `<base>` arg must be a commit on `master`.

* To list/start support branches, use:
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
