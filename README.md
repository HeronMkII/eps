# eps

EPS codebase.

## Installing lib-common

When cloning this repository for the first time, you will have to download and
build lib-common from scratch. To do this, run the following commands from the
root directory of this project:

```
$ git submodule update --init lib-common
$ cd lib-common
$ make
$ cd ..
```

The first command initializes the lib-common library. The next three commands
rebuild it.
