# VTIL-Samples

This repository contains a number of samples to get you started with VTIL's API.

## Building (Visual Studio)

Open the project as folder in Visual Studio 2019 and let it generate the CMake files. Once the generation has ended, you can launch the examples.

## Building (MacOS)

First install LLVM 10 and Ninja:

```sh
brew install llvm@10 ninja
```

Then set your compiler environment to use libc++ and brew's llvm:

```sh
export CC="/usr/local/opt/llvm/bin/clang"
export CXX="/usr/local/opt/llvm/bin/clang++"
export LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
```

Then clone the repo and build:

```sh
git clone https://github.com/vtil-project/VTIL-Samples
cd VTIL-Samples
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Examples

### Logger

This example demonstrates VTIL's versatile logging capabilities.

### Intro

A very basic example of VTIL's `basic_block` functionality. Based on https://0xnobody.github.io/devirtualization-intro/

### Simplification

A short introduction into assembly simplification using VTIL's optimizer passes.

## Brainfuck

A lifter for [brainfuck](https://en.wikipedia.org/wiki/Brainfuck) and an example VM to execute the lifted brainfuck.
