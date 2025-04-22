# Soft Body Simulation (slimeEngine)

## How to Compile and Run
**On UNIX systems**
```shell
mkdir build
cd build
cmake ../
make
```

**On Windows (Win32):**
- Use **Visual Studio** to open the project solution and build.

## How to Use the Executable
**On UNIX systems**
From root folder,
```shell
cd build
./slimeEngine
```

**On Windows (Win32)**
After building, run:
```
slimeEngine\build\Debug\slimeEngine.exe
```

### Selecting the Model
Available objects:
```
0 - pudding
1 - sphere
2 - bunny
3 - tetrahedron
4 - cube
```
By default, the model is **pudding** (index 0).  
To select another model, pass its index as an argument:
```
.\slimeEngine.exe 1
```
(Loads the **sphere**.)
Or on UNIX:
```shell
./slimeEngine 1
```

## User Interface and Controls

### Camera Controls
- **Mouse Movement**  
  - Press **L**: Lock the mouse to control camera yaw and pitch (first-person view).
  - Press **K**: Unlock the mouse for normal cursor movement (adjust parameters in ImGui).
- **W / A / S / D keys** — Move camera forward, left, backward, and right.

### Object Interaction
- **Aim** at the soft body and press **G** to grab and drag a vertex.
- **Press R** to release the selected vertex.
- **Selection** uses ray tracing from the center of the screen to hit soft body triangles.

### ImGui Control Panel
A real-time panel (built with ImGui) allows live tuning:
- **Edge Compliance** — Softness of edges: 0.01f to 0.2f
- **Volume Compliance** — Resistance to volume change: 0.01f to 0.2f
- **Substeps** — Number of times the constraints are solved:  1 to 50
- **Reset Button** — Resets the model (drops it from a height of 5.0f).
- **Lift Button** — Moves the entire soft body upwards while holding the button  


## Problems We Encountered
- Intel graphics drivers may not fully support **OpenGL 3.3**, leading to compatibility issues.
