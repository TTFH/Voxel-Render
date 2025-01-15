# Voxel Rendering Engine

This is a simple voxel rendering engine written in C++ and OpenGL. It can render voxels using conventional rasterization or ray tracing.

> [!Important]
> This program is cross-platform and has been tested on Windows 7, 11 and Linux.

## Features

- Scene loading: Load scenes from XML files.
- Geometry support:
	- Voxels (cubes)
	- Voxagons (rectangular prisms)
	- Water (2D triangle fan)
	- Ropes (line strips)
	- Triangular meshes
- Greedy Meshing Algorithm:  
Optionally generates triangular meshes for voxels using Mikola Lysenko’s [Meshing in a Minecraft Game (Part 2)](https://0fps.net/2012/07/07/meshing-minecraft-part-2/).
- Hexagonal voxel support:  
Renders hexagonal prisms instead of cubes and aligns them in all three orientations.
- GUI Integration:  
Includes support for Dear ImGui, the GUI library used in applications like Teardown and GTA VI.

> [!Note]
> This engine is a proof-of-concept and may not be optimized for production environments.

## Dependencies

The engine uses the following libraries:

- [GLAD](https://glad.dav1d.de/) OpenGL function loading
- [GLFW](https://www.glfw.org/) Window management.
- [GLM](https://glm.g-truc.net/0.9.9/index.html) Vector and matrix math.
- [stb](https://github.com/nothings/stb) Texture loading and screenshot saving.
- [tinyxml2](https://github.com/leethomason/tinyxml2) XML file handling.
- [imgui](https://github.com/ocornut/imgui) Graphical user interface.

> [!Caution]
> Make sure all dependencies are installed and accessible before attempting to compile the project.

## How to Compile

- Open a terminal in the project folder.
- Run the following command:

```bash
    make
```

## Example Scenes

Render the included example scenes using these commands:

- Render the example scene:
```bash
./vox_render example/
```
- Render the castle scene:

	1. Extract castle.7z.
	2. Run:
```bash
    ./vox_render main.xml
```
- Render a converted Teardown map:
```bash
	./vox_render marina_sandbox/main.xml
```
> [!Tip]
> Use the F10 key during rendering to save screenshots, or F11 to toggle fullscreen.

## Controls

- Camera Movement:
	- W, A, S, D, Space, Ctrl: Move the camera.
	- Hold Shift: Move the camera faster.
	- Hold Right Mouse Button and drag: Look around.
- Lighting Controls:
	- Arrow Keys, Q, E: Move the light source.
- Screenshots:
	- Press F10 to save a screenshot.
- Fullscreen Toggle:
	- Press F11 to toggle fullscreen.

## Engine Rendering Example

> [!Warning]
> For large scenes, the rendering process may become CPU/GPU intensive. Ensure your system has sufficient resources.

## Screenshots
![img0](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img0.png)
![img1](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img1.png)
![img2](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img2.png)
![img3](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img3.png)
