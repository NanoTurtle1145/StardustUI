# Install and Build

## Clone

```bash
git clone https://github.com/xingji-studio/StardustUI.git
cd StardustUI
```

## Platform backends

StardustUI currently builds for:

- `xj380`
- `linux`
- `windows`

The top-level `Makefile` selects a platform automatically when possible, and you can also override it with `PLATFORM=...`.

## Linux

The Linux backend currently uses SDL2 and SDL_ttf.

On Arch Linux:

```bash
sudo pacman -S sdl2 sdl2_ttf
make PLATFORM=linux
```

This produces:

```text
build/libStardustUI.a
```

## Windows

You can build the Windows backend with MinGW:

```bash
make PLATFORM=windows CXX=x86_64-w64-mingw32-g++
```

## XJ380

Build with:

```bash
make PLATFORM=xj380
```

This requires the XJ380 toolchain and related objects used by the repository's `Makefile`.

## Platform detection

If you do not define a platform macro manually, `settings.hpp` currently uses:

- `_WIN32` -> `STARDUSTUI_WINDOWS`
- `__linux__` -> `STARDUSTUI_LINUX`
- otherwise -> `XJ380`

## Build the example

The example lives in `examples/helloworld`.

Linux:

```bash
cd examples/helloworld
make PLATFORM=linux
./build/linux/helloworld
```

Windows:

```bash
cd examples/helloworld
make PLATFORM=windows CXX=x86_64-w64-mingw32-g++
```

XJ380:

```bash
cd examples/helloworld
make PLATFORM=xj380
```

## Related pages

- [Quick Start](./quickstart.md)
- [Create a Window](./create_window.md)
- [Style System](./style.md)
