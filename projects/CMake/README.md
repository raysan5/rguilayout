# rGuiLayout CMake Definitions

This provides CMake definition files for rGuiLayout.

## Usage

1. create build dir
```bash
mkdir build
```

2. configure cmake
```bash
cmake -Sprojects/CMAKE -Bbuild
```

3. compile the project
```bash
cmake --build build --config Release --target all
```

## HTML5 target

Replace step 2 with:
```bash
cmake -Sprojects/CMAKE -Bbuild  -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
```

The exact toolchain location may vary based on installation.