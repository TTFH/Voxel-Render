# Voxel Rendering Engine

This is a simple voxel rendering engine written in C++ and OpenGL, it can render voxels using conventional rasterization or raytracing.  

This program is cross-platform and has been tested on Windows 7, 10, 11 and Linux and MacOS. It can load a scene from a XML file and render voxels (cubes), voxagons (rectangular prisms), water (2D triangle fan), ropes (line strips) and triangular meshes (wait, what?).  
It optionally uses a greedy meshing algorithm to generate the triangular meshes for the voxels, based on the post [Meshing in a Minecraft Game (Part 2)](https://0fps.net/2012/07/07/meshing-minecraft-part-2/) by Mikola Lysenko.  
It can also render voxels like hexagonal prisms instead of cubes, and align the hexagons on all 3 orientations.  
It supports Dear ImGui, the amazing GUI library used by Teardown and GTA VI.  

It make use of the following libraries:  
-   [GLAD](https://glad.dav1d.de/) for OpenGL function loading.
-   [GLFW](https://www.glfw.org/) for window management.
-   [GLM](https://glm.g-truc.net/0.9.9/index.html) for vector and matrix math.
-   [stb](https://github.com/nothings/stb) for loading textures and saving screenshots.
-   [tinyxml2](https://github.com/leethomason/tinyxml2) for loading XML files.
-   [imgui](https://github.com/ocornut/imgui) for the Graphical User interface.

### How to compile

Open a command prompt in the project folder and run the command: `make`

Render the example scene:  
`./vox_render example/`

Render the castle scene:  
Extract castle.7z  
`./vox_render main.xml`

Render a converted Teardown map:  
`./vox_render marina_sandbox/main.xml`

## Controls

WASD, Space & Ctrl to move the camera  
Hold Shift to move the camera faster  
Hold Right Mouse Button and drag to look around  
Arrow keys, Q & E to move the light  
F10 save screenshot  
F11 toggle fullscreen

![img0](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img0.png)
![img1](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img1.png)
![img2](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img2.png)
![img3](https://raw.githubusercontent.com/TTFH/Voxel-Render/main/screenshots/img3.png)
