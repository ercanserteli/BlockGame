# BlockGame

This is a block game project written in C++ with SDL and OpenGL. It is more of a practice in graphics and game engine development than an actual game. 

## Features:

- The game is made of two pieces, GameCode is built as a dynamic library and GameBase is an executable that loads it as a library. GameCode can be modified and recompiled, and GameBase will automatically hot-reload the new game code seamlessly.
- The game is auto saved and loaded.
- There is controller support in addition to the default keyboard - mouse scheme.

## Controls:

- WASD to move, mouse for camera movement
- E to decrease FOV, Q to increase FOV
- F to switch between place mode and throw mode
- RMB to place blocks in place mode
- RMB to pickup (on click) and throw (on release) blocks in throw mode
- LMB to break blocks
- Mouse wheel to select block types
- Space to jump
- Left shift to jetpack
- F2 to screenshot
- F3 to toggle graphics debug visuals
- F4 to toggle sun speed boost
- ESC to quit game
- In non-release builds, R to start recording and R again to start/end replaying

## Build

- Make a build directory: `mkdir build`
- Initialize cmake in build directory: `cd build && cmake ..`
- Either build with cmake: `cmake --build . --config Internal` or open the generated solution and build in Visual Studio.
    - There are three configs: Debug, Release and Internal. Internal is optimized but has debug features enabled, so it is best for development.
