# Voxel Rendering Engine

This is a simple voxel rendering engine written in C++ and OpenGL, it uses conventional rasterization instead of raytracing.

This program is cross-platform and has been tested on Windows and Linux. It can load a scene from a XML file and render voxels (cubes), voxagons (rectangular prisms), water (2D triangle fan) and ropes (line strips).  
It uses a greedy meshing algorithm to generate the triangular meshes for the voxels, based on the post [Meshing in a Minecraft Game (Part 2)](https://0fps.net/2012/07/07/meshing-minecraft-part-2/) by Mikola Lysenko.

It make use of the next libraries:

-   [GLAD](https://glad.dav1d.de/) for OpenGL function loading.
-   [GLFW](https://www.glfw.org/) for window management.
-   [GLM](https://glm.g-truc.net/0.9.9/index.html) for vector and matrix math.
-   [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) for loading textures.
-   [tinyxml2](https://github.com/leethomason/tinyxml2) for loading XML files.

### How to compile

Open a command prompt in the project folder and run the command: `make`

![img2](https://github.com/TTFH/Voxel-Render/blob/e57db0be0b18ef2de9a2b6e52d53666dd5d5685e/img2.png)
