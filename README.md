# Soft Body Simulation (slimeEngine)

## How to Compile and Run
**Modify `CMakeLists.txt`:**
```cmake
set(GLFW3_LIBRARY "slimeEngine/libs/glfw3.lib")
set(GLFW3_INCLUDE "slimeEngine/include/GLFW")
set(ASSIMP_LIBRARY "slimeEngine/libs/assimp/assimp-vc143-mtd.lib")
```

**On Windows (Win32):**
- Use **Visual Studio** to open the project solution and build.


## How to Use the Executable
After building, run:
```
slimeEngine\build\Debug\slimeEngine.exe
```

### Selecting the Object
Available objects:
```
0 - pudding
1 - sphere
2 - bunny
3 - tetrahedron
4 - cube
```

By default, the model used is **pudding** (index 0).

To choose a different model, pass the object index as a command-line argument.  
For example, to select **sphere**:
```
.\slimeEngine.exe 1
```

This will load the **sphere** model instead.


## Problems We Encountered
- Intel graphics drivers may not fully support **OpenGL 3.3**, leading to compatibility issues.
