# meshing
An interactive application for sculpting in 3D. 

Distance functions defining the surface of a shape can be added additively and subtractively to form complex shapes.
Functions are added to an octree which causes affected areas to be re-evaluated.
Function evaluation consists of iterating over a hierarchical grid to find distances smaller than some threshold.
Zero-crossing points are collected and uploaded to an openGL vertex buffer, then drawn.

__[Video](https://www.youtube.com/watch?v=L7l1_QjAOaE)__

__Controls:__
* mouse movement: look around
* left click: apply brush additively
* right click: apply brush subtractively
* 1 key: toggle between sphere and cube brushes
* Up Arrow, Down Arrow: increase or decrease brush size
* 3 and 4 keys: increase and decrease brush fidelity
* WS: forward and backward
* AD: left and right
* left shift, space: down and up

__Linux Dependencies:__
* OpenGL 4.3
* glew
* glfw3
* glm
* clang
* make
  
__Linux Building:__
* make -j release

__Linux Running:__
* make run

__Windows Building:__
* requires VS2015 installed at default location, or an x64 cmd environment
* requires Windows 7.1A SDK installed in default location for OpenGL32.lib
* loadenv.bat
* build.bat
* run.bat

__[Windows Binary](https://github.com/gheshu/meshing/tree/master/build)__

![alt tag](http://i.imgur.com/fyDl3kW.png)
