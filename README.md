# Requirements
- Make
- CMake
- A C++ compiler

# Building
You will find the final binaries in the <build_dir>/shippable directory.

Ignore the other binaries, only the ones in the shippable directory are the final ones.

The following instructions will assume the <build_dir> is named `build`.

If building from inside an IDE, make sure the `shippable` target is selected.

# Windows
On Windows you will need a GCC compiler, you can use Cygwin or MinGW.

## Commands to execute
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release --target shippable
```

Once again, the final binaries will be in the `build/shippable` directory.
