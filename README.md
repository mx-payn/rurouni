# About
Darkness and void... But there is light!

# Building / Installing

Targets to build/install (possible `<target>` in following commands):
- `rurouni-editor`
- `rurouni-runtime`

The executables and libraries depend on the build and install locations
(e.g. when building, data is located in `apps/<target>/data`, when installed probably in `/usr/local/share/<target>`)
Use the provided presets to build or install targets, which will automatically
set the right paths:

```sh
# build
cmake --preset=build
cmake --build build --target <target>
```

```sh
# install
cmake --preset=install
cmake --build build --target <target>
cmake --install build/<target>
```

More options and documentation will be found in this link (when its done...)
