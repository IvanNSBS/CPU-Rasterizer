# CPU-Rasterizer
## Repo with implementation of camera, rasterization and other algorithms for basic Computer Graphics, using CPU instead of GPU using the SDL2 library

## Compile Commands
- g++ sdl_test.cpp -IC:\mingw64\include -LC:\mingw64\lib -g -O3 -w -lmingw32 -lSDL2main -lSDL2 -o Renderer.exe

## Camera Controls
- WASD for movement. 
- E/Q to move camera up and down, respectively.
- Control Look At by right-mouse clicking and moving mouse

## Example 1:
![](./result1.png)

## Example 2:
![](./result2.png)


## Problems:
- Camera Rotation might be too sensible for current camera setup
- Camera Rotation might be problematic at extreme angles because Quaternions weren't used. To fix some problems, placeholders were used but they didn't account for those cases.
- Triangle Clipping wasn't implemented, only lines. Source code must be modified and recompiled to render using lines instead of triangles.
