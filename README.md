# LT's Aim Train 21

OpenGL 2.1 aim trainer.

## Controls

- Mouse: Look
- Left Click: Shoot
- Scroll: Target size
- -/+: Volume control
- F11: Fullscreen
- ESC: Exit

## Build

### Linux
```bash
sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
make
./ltat21
```

### Windows (MinGW)
```bash
make -f Makefile.mingw
ltat21.exe
```

### macOS
```bash
brew install glfw
make
./ltat21
```

## Requirements

- C compiler
- GLFW 3.x
- OpenGL 2.1

## License

Restrictive source-available license. Personal educational use only. See LICENSE file.