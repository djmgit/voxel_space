# Voxel Space

This is an attempt at implementing voxel space from scratch using Raylib. The player or the camera can move forward, rotate, lift up and down, tilt left, right, front and back.
I have also tried to add fog effect using two different methods.
The rendered worlds look something like this:

https://github.com/djmgit/voxel_space/assets/16368427/20eb69ed-96bf-4ddb-a4a2-85214f3da049

## What is Voxel space?

Voxel space is an old, extremely popular, first of its kind algorithm which was used for rendering pseudo 3D terrains. It was created by Novalogic developer 
Kyle Freeman, who also <a href="https://patents.google.com/patent/US6020893"> patented </a> it, and was used to create their signature game Comanche.
I wont go into the details of voxel space here since there are plenty of articles and tutorials online. In short voxel space renders graphics pixel by
pixel on screen while casting rays from the camera location. Two things which are pre-requisite for voxel space is a color map which defines the color
of each pixel in the terrain and a height map which defines how high on screen each pixel is.

## How to build and run

If you are running on linux, build and install raylib library for your distribution following the official <a href="https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux">documentation</a>.
If you are on linux you can use the provided raylib library and header files in lib folder. I have also provided raygui.h in this repository itself.

For building and running you can do the following:

- Clone this repo and open it in terminal.
- Run ```make```
- Run ```make run``` for running the project.

You should be presented with raylib window.

## Adding fog

I gave a shot at adding fog to the scene. Fog is a useful tool to hide clipping of the graphics at the far end of the z axis. I tried to implement two different fog
<a href="https://learn.microsoft.com/en-us/windows/win32/direct3d9/fog-formulas">implementations</a> :

- Linear fog: Which takes into consideration where the fog starts and ends.
- Exponential fog: Which applies a damping factor on the original color of the pixel as the z distance from camera increases.

You can see how both fog implementations look by switching between them on the GUI and playing with the tuning parameters.

## References

- Video tutorial by Pikuma: https://www.youtube.com/watch?v=bQBY9BM9g_Y&t=3270s . He has explained Voxel space with great detail, one of the best videos for voxel space explanation out on Internet.
- Voxel space expanation by Sebastian Mackle: https://github.com/s-macke/VoxelSpace .
- Codermind article: https://web.archive.org/web/20131113094653/http://www.codermind.com/articles/Voxel-terrain-engine-building-the-terrain.html


