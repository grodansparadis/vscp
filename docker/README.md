# Docker Images 

This directory contains Docker files for Mosquitto.

The `1.5` directory contains the latest version of Mosquitto for
that series, and provide the basis of the official image.

`1.4.12` is the version using Alpine packaged Mosquitto, which will be removed
at the next minor release.

The `generic` directory contains a generic Dockerfile that can be used to build
arbitrary versions of Mosquitto based on the released tarballs as follows:

```
cd generic
docker build -t eclipse-mosquitto:1.5.1 --build-arg VERSION="1.5.1" .
docker run --rm -it eclipse-mosquitto:1.5.1
```

The `local` directory can be used to build an image based on the files in the
working directory by using `make localdocker` from the root of the repository.
