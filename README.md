# Penguin Engine

This is a personal project of mine to create a standalone software renderer without the usage of libraries like OpenGL, SDL2, etc. The only third party library currently in use is `zlib`.

Why's it called Penguin? Because that's my favorite animal, that's all!

![](https://github.com/thomascswalker/PenguinEngine/blob/dff8ced27bb88a681dc4d9395a7dadfb33035b27/example.gif)

## Requirements
- C++ 20
- Visual Studio 2022
- CMake 3.26
- Git

> [!NOTE]
> Currently this only runs on Windows. In the future I plan to add support for Linux and MacOS.

## Building the project from source
1. Double-click the `Setup.sh` file in the root of this repository. This will run CMake and build the project.
2. Open the `PenguinEngine.sln` solution inside the `Build` directory.
