# Sora Branch Notes

On Linux to build not using SDL system dll do the following:

```sh
$ git clone --branch release-2.32.2 https://github.com/libsdl-org/SDL.git SDL2
$ git clone --branch release-2.24.0 https://github.com/libsdl-org/SDL_ttf.git SDL2_ttf
```

```sh
$ mkdir build
$ cd build
$ cmake -DUSE_SYSTEM_LIBSDL2=OFF -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . -- -j 10
```

The `CMAKE_BUILD_TYPE` to properly create SDL2 generated config because `$<CONFIG>` generator some how empty.
 
