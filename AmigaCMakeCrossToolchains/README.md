# AmigaCMakeCrossToolchains

**Note: This fork is just me playing around making this work better for me cross compiling in a MacOS/Clion environment. vbcc is a total hack job at this point, don't use.**

This repo contains CMake toolchain files for configuring CMake-based projects to build binaries for Amiga-like systems.

Typical usage is as follows:

```shell
cd /project/to/build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchainFile.cmake -D<TOOLCHAIN_FILE_SPECIFIC_DEFINES>
```

