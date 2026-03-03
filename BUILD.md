# Building tek-game-runtime

tek-game-runtime must be built in [MSYS2](https://www.msys2.org/), preferably with [TEK-X86_64 environment](https://github.com/teknology-hub/MinGW-env) that is tuned for building TEK software.

## 1. Install requirements

```sh
pacman -S base-devel git mingw-w64-tek-x86_64-toolchain mingw-w64-tek-x86_64-rapidjson mingw-w64-tek-x86_64-tek-steamclient
```

## 2. Get source code

Clone this repository:
```sh
git clone https://github.com/teknology-hub/tek-game-runtime.git
cd tek-game-runtime
```
, or download a point release e.g.
```sh
curl -LOJ https://github.com/teknology-hub/tek-game-runtime/releases/download/vX.Y.Z/tek-game-runtime-X.Y.Z.tar.gz`
tar -xzf tek-game-runtime-X.Y.Z.tar.gz
cd tek-game-runtime-X.Y.Z
```

## 3. Setup build directory

At this stage you can set various build options, which are described in [Meson documentation](https://mesonbuild.com/Commands.html#setup). Release builds use the following setup:
```sh
CXXFLAGS="-pipe -fomit-frame-pointer" meson setup build --buildtype debugoptimized -Db_lto=true -Db_lto_mode=thin -Db_ndebug=true
```

## 4. Compile the library

```sh
meson compile -C build
```
This will produce `libtek-game-runtime.dll` and `libtek-game-runtime.pdb` in the build directory that you can use now. You may also want to strip the DLL of remaining debug/meta information:
```sh
strip --strip-unneeded libtek-game-runtime.dll
```
