# Rurouni
A 2D rogue-like ASCII Engine for educational games on programming languages.
Rurouni is developed by me as a practice and learning experience for my university studies.

# Requirements
* CMake 3.17 or better
* A C++17 compatible compiler
* Git
* Doxygen (optional)

# Building
Rurouni has dependencies added as git submodules, so be sure to clone recursively:

``` bash
git clone --recurse-submodules https://github.com/mx-payn/rurouni.git
cd rurouni
```

For git version 2.8+ you can add the `-j8` flag, which fetches up to 8 submodules at a time in parallel.

To configure:

```bash
cmake -S . -B build
```

Add `-GNinja` if you have Ninja.

To build:

```bash
cmake --build build
```

To test (`--target` can be written as `-t` in CMake 3.15+):

```bash
cmake --build build --target test
```

To build docs (requires Doxygen, output in `build/docs/html`):

```bash
cmake --build build --target docs
```

To use an IDE, such as Xcode:

```bash
cmake -S . -B xbuild -GXcode
cmake --open xbuild
```
