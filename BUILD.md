# Building tek-game-runtime

tek-game-runtime must be built in [MSYS2](https://www.msys2.org/), preferably in its CLANG64 environment, which is assumed to be used in the following steps

## 1. Install requirements

```sh
pacman -S base-devel git mingw-w64-clang-x86_64-cc mingw-w64-clang-x86_64-meson mingw-w64-clang-x86_64-rapidjson
```
[tek-steamclient](https://github.com/teknology-hub/tek-steamclient) headers must also be present. There is no MSYS2 package for it yet, but you can just copy the `include/tek-steamclient` directory from its repository to `/clang64/include/`:
```sh
git clone https://github.com/teknology-hub/tek-steamclient.git
cp -r tek-steamclient/include/tek-steamclient /clang64/include/
rm -rf tek-steamclient
```

## 2. Get source code

Clone this repository:
```sh
git clone https://github.com/teknology-hub/tek-game-runtime.git
cd tek-game-runtime
```
, or download a point release e.g.
```sh
curl -LOJ https://github.com/teknology-hub/tek-game-runtime/releases/download/v1.0.0/tek-game-runtime-1.0.0.tar.gz`
tar -xzf tek-game-runtime-1.0.0.tar.gz
cd tek-game-runtime-1.0.0
```

## 3. Setup build directory

At this stage you can set various build options, which are described in [Meson documentation](https://mesonbuild.com/Commands.html#setup). Release builds use the following setup:
```sh
CXXFLAGS="-pipe -fomit-frame-pointer" meson setup build --buildtype debugoptimized -Dprefer_static=true -Db_lto=true -Db_lto_mode=thin -Db_ndebug=true
```

## 4. Compile the library

```sh
meson compile -C build
```
This will produce `libtek-game-runtime.dll` and `libtek-game-runtime.pdb` in the build directory that you can use now. You may also want to strip the DLL of remaining debug/meta information:
```sh
strip --strip-unneeded libtek-game-runtime.dll
```
