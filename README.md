SHMUP :
=======

This is the source code of "SHMUP" a 3D Shoot 'em up that I wrote in 2009.
It is very inspired of Treasure Ikaruga, the engine runs on iOS, Android, Windows and MacOS X.
It has also been ported to Linux by "xevz".

Technical side :
================

It is written in ANSI C with wrapper for the specific platforms, rendition is done with OpenGL ES 1.1 and there is even an experimental rendition path based on OpenGL ES 2.0 that uses the dEngine source ( which can be found here: http://fabiensanglard.net/dEngineSourceCodeRelease/index.php).

Enjoy,

Fabien Sanglard 

Windows + GLFW Build Guide
==========================

Test env : VS2013

1. git submodule init
2. git submodule update
3. cd engine\glfw
4. cmake CMakeLists.txt
5. open Shmup.sln, start project is ```shmup```. Build.
6. copy ```glewd.dll``` from ```engine\glfw\bin\Debug``` to ```engine\glfw\Debug```.
7. project properties -> debugging -> working directory is ```$(ProjectDir)\..\..\..```.

* control : Mouse
* quit : ESC

