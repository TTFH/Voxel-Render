# Voxel Rendering Engine

This is a simple voxel rendering engine written in C++ and OpenGL, it uses conventional rasterization instead of raytracing.

This program is cross-platform and has been tested on Windows and Linux. It can load a scene from a XML file and render voxels (cubes), voxagons (rectangular prisms), transparent water (2D triangle fan), ropes (line strips) and triangular meshes (wait, what?).  
It optionally uses a greedy meshing algorithm to generate the triangular meshes for the voxels, based on the post [Meshing in a Minecraft Game (Part 2)](https://0fps.net/2012/07/07/meshing-minecraft-part-2/) by Mikola Lysenko.

It make use of the following libraries:

-   [GLAD](https://glad.dav1d.de/) for OpenGL function loading.
-   [GLFW](https://www.glfw.org/) for window management.
-   [GLM](https://glm.g-truc.net/0.9.9/index.html) for vector and matrix math.
-   [stb](https://github.com/nothings/stb) for loading textures and saving screenshots.
-   [tinyxml2](https://github.com/leethomason/tinyxml2) for loading XML files.

### How to compile

Open a command prompt in the project folder and run the command: `make`

Render the example scene:  
`./vox_render example`

Render the castle scene:  
Extract castle.7z  
`./vox_render main.xml`

Render a converted Teardown map:  
`./vox_render marina_sandbox/main.xml`

![img0](https://github.com/TTFH/Voxel-Render/blob/0975fe6f830e005a112d559c4a423612f18e579f/img0.png)
