# Project Description

This is template CMake, FreeGLUT and GLAD project for working with legacy OpenGL.

It should theoretically work on every platfom possible.

# Building

0. Install CMake. Minimum version should be 3.10.

1. Configure cmake build directory. It is going to produce binary files to specified path.

```cmake -S . -B Build```

2. Build project using this command.

```cmake --build Build```

3. Run program. (Varies between different OS)

```./Build/OpenGL-Application```
