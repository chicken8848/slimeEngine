# Soft Body Simulation (slimeEngine)

## How to Compile and Run
**Modify `CMakeLists.txt`:**
```cmake
set(GLFW3_LIBRARY "slimeEngine/libs/glfw3.lib")
set(GLFW3_INCLUDE "slimeEngine/include/GLFW")
set(ASSIMP_LIBRARY "slimeEngine/libs/assimp/assimp-vc143-mtd.lib")
```

**On Windows:**
- Use **Visual Studio** to open the project and build.

## How to Use the Executable
After building, run:
```
slimeEngine\build\Debug\slimeEngine.exe
```

## Problems We Encountered
- Intel graphics drivers may not fully support **OpenGL 3.3**, leading to compatibility issues.
