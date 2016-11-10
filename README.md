# simon-speck-supercop

Fast implementations of the SIMON and SPECK lightweight block ciphers for the
SUPERCOP benchmark toolkit.

These implementations use counter mode to work as stream ciphers and are
available in the following directories:

* SIMON
    * 64/96    - [crypto_stream/simon6496ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/simon6496ctr)
    * 64/128   - [crypto_stream/simon64128ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/simon64128ctr)
    * 128/128  - [crypto_stream/simon128128ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/simon128128ctr)
    * 128/192  - [crypto_stream/simon128192ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/simon128192ctr)
    * 128/256  - [crypto_stream/simon128256ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/simon128256ctr)
* SPECK
    * 64/96    - [crypto_stream/speck6496ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/speck6496ctr)
    * 64/128   - [crypto_stream/speck64128ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/speck64128ctr)
    * 128/128  - [crypto_stream/speck128128ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/speck128128ctr)
    * 128/192  - [crypto_stream/speck128192ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/speck128192ctr)
    * 128/256  - [crypto_stream/speck128256ctr/](https://github.com/iadgov/simon-speck-supercop/tree/master/crypto_stream/speck128256ctr)

Each algorithm directory contains subdirectories for particular SIMD
instruction sets, currently x86 CPUs with SSE4.2, AVX2, or 32-bit ARM CPUs with
NEON.

## Branches

The supercop branch is a copy of the latest SUPERCOP release. The simon-speck
branch contains any updates to our SIMON and SPECK implementations since the
last SUPERCOP release. The master branch includes this and other documentation
merged with the supercop and simon-speck branches.

## Getting the Code

There are a few ways to get these implementations without having to clone the
entire repository.

### Downloading only SIMON and SPECK

Archives are available containing just the SIMON and SPECK directories in
[.tar.xz - 17 KB](https://iadgov.github.io/simon-speck/implementations/code/supercop/simon-speck.tar.xz),
[.tar.gz - 50 KB](https://iadgov.github.io/simon-speck/implementations/code/supercop/simon-speck.tar.gz),
and [.zip - 167 KB](https://iadgov.github.io/simon-speck/implementations/code/supercop/simon-speck.zip)
format.

### Patches

GitHub will provide a patch covering the changes between two branches. For
example, a patch to update the latest SUPERCOP with our changes is available at
the following:

https://github.com/iadgov/simon-speck-supercop/compare/supercop...simon-speck.diff

This can be downloaded and and applied with commands like the following:

    cd supercop-20161026
    curl https://github.com/iadgov/simon-speck-supercop/compare/supercop...simon-speck.diff -o simon-speck.diff
    patch -p1 < simon-speck.diff

or

    curl https://github.com/iadgov/simon-speck-supercop/compare/supercop...simon-speck.diff | patch -p1

## Legal

Refer to individual files and directories for the license of SUPERCOP
components. For the SIMON and SPECK code see [LICENSE](./LICENSE.md) and
[DISCLAIMER](./DISCLAIMER.md).
