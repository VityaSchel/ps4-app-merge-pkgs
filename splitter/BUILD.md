# How to compile for each platform on macos

1. Install `mingw-w64` and `FiloSottile/musl-cross/musl-cross` via homebrew
2. Go to splitter directory
3. Run these command:

```sh
rm -rf build build-windows build-linux
touch build/.gitkeep && touch build-windows/.gitkeep && touch build-linux/.gitkeep
mkdir build && cd $_
cmake ..
make
cd ..
mkdir build-windows && cd $_
cmake .. -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
make
cd ..
mkdir build-linux && cd $_
cmake .. -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_C_COMPILER=x86_64-linux-musl-gcc -DCMAKE_CXX_COMPILER=x86_64-linux-musl-g++
make
```