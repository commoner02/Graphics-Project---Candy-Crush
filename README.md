# 3D Geometric Shape-Match (4-Tier Stepped Pyramid)

A high-performance OpenGL project demonstrating 100% procedural geometric primitive generation using pure non-overlapping triangles (`GL_TRIANGLES`), 3D raycast picking, and geometric shape-matching across a 4-tier stepped pyramid with vibrant colored wireframe and architectural clay shading.

---

## File Structure

```
candy_pyramid/
├── Makefile                    # Fast build: `make run`
├── CMakeLists.txt              # Standard CMake build script
├── build.sh                    # Direct g++ compilation script
├── README.md                   # Project overview & controls
├── CODEBASE.md                 # Monolithic codebase documentation
│
├── include/
│   ├── Mesh.h                  # GPU buffer wrapper (VAO, VBO, EBO)
│   ├── Primitives.h            # 4 Procedural shapes (Sphere, Cylinder, Cube, Diamond)
│   ├── Shader.h                # GLSL shader program compiler & loader
│   ├── Camera.h                # 360 orbital camera with 3D raycasting
│   ├── Shape.h                 # ShapeTile struct & 3D arc-hop trajectories
│   └── Board.h                 # Stepped pyramid layout, picking & match-3 rules
│
├── src/
│   ├── Mesh.cpp                # Buffer allocation & indexed drawing
│   ├── Primitives.cpp          # Exact parametric triangulation math
│   ├── Shader.cpp              # Shader compilation and uniform binding
│   ├── Camera.cpp              # View matrix & 3D screen ray unprojection
│   ├── Shape.cpp               # ShapeTile wireframe rendering
│   ├── Board.cpp               # 3D raycast picking, match detection, gravity drop
│   └── main.cpp                # GLFW window setup, input callbacks, and render loop
│
└── shaders/
    ├── wireframe.vert          # Vertex shader (pos, normal, MVP transforms)
    └── wireframe.frag          # Fragment shader (wireframe outline & monochrome clay)
```

---

## How to Play & Interactive Controls

| Control                | Action                     | Details                                                                                                           |
| :--------------------- | :------------------------- | :---------------------------------------------------------------------------------------------------------------- |
| **Left Click**   | **Select & Swap**    | Unprojects 2D screen click into 3D world space. Click an adjacent shape to swap in a 3D parabolic arc.            |
| **Match-3**      | **Shape Matching**   | Align 3 identical**3D Shapes** (Sphere, Cylinder, Cube, Diamond) in a row to crush & trigger gravity drops. |
| **`E` Key**    | **Exploded View**    | Smoothly separates the 4 tiers vertically along $Y$ to inspect the internal `GL_TRIANGLES` grid.               |
| **`W` Key**    | **Wireframe Toggle** | Toggles between **Vibrant Colored Wireframe** (default) and **Architectural Solid Shading**.                 |
| **`P` Key**    | **Projection**       | Switches between **Perspective ($45^\circ$)** and **Orthographic** blueprint projection.             |
| **`M` Key**    | **Matrix Inspection**| Dumps full 4x4 Model, View, Projection, and MVP matrices to the terminal for mathematical review. |
| **`Spacebar`** | **Auto-Orbit**       | Pauses or resumes 360 continuous camera rotation.                                                                 |
| **`R` Key**    | **Reshuffle**        | Re-randomizes the board and resets score.                                                                         |
| **Right Drag**   | **Manual Orbit**     | Click and drag right mouse button to rotate around the pyramid freely.                                            |
| **Scroll Wheel** | **Zoom**             | Smooth distance zoom in and out.                                                                                  |
| **`Esc` Key**  | **Exit**             | Quits the application cleanly.                                                                                    |

---

## How to Compile & Run on Fedora Linux

```bash
cd candy_pyramid
make run
```

Or via CMake:

```bash
mkdir -p build && cd build && cmake .. && make -j$(nproc) && ./candy_pyramid
```

Or via script:

```bash
./build.sh
```
