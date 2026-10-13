# 3D Candy Crush Pyramid: Stepped Colosseum Match-3 Engine

An advanced, hardware-accelerated **OpenGL 3.3 Core Profile** graphics application and match-3 puzzle engine built in **C++17**. The project features 100% procedural parametric confectionery geometry synthesis using pure non-overlapping triangles (`GL_TRIANGLES`), analytical Blinn-Phong and Gouraud illumination, screen-to-world 3D raycasting, a non-Euclidean stepped colosseum topology, and an immediate-mode 2D vector HUD.

Developed for **CSE 4102: Computer Graphics and Image Processing Laboratory**, Department of Computer Science and Engineering, **Khulna University of Engineering and Technology (KUET)**.

---

## Author Information

- **Developer:** Sree Shuvo Kumar Joy
- **Roll:** 2107116
- **Year / Term:** 4th Year, 1st Term
- **Department:** Department of Computer Science and Engineering (CSE)
- **Institution:** Khulna University of Engineering and Technology (KUET)

---

## Key Technical Highlights

1. **100% Procedural Parametric Geometry (Zero 3D Asset Files):**
   - Synthesizes all confections at runtime using Frenet-Serret frames, Gaussian equatorial flanges, and fractional-exponent superquadrics without importing `.obj` or `.fbx` files.
   - Calculates exact analytical surface normal vectors ($\mathbf{N}$) via vector cross-products and partial derivatives.
2. **Stepped Colosseum Non-Euclidean Board (160 Active Candies):**
   - **Base Tier (Layer 0):** $8 \times 8$ footprint with 84 candies wrapping 4 vertical facade walls across 3 elevation rings.
   - **Middle Tier (Layer 1):** $6 \times 6$ footprint with 60 candies wrapping 4 vertical facade walls across 3 elevation rings.
   - **Summit Tier (Layer 2):** $4 \times 4$ planar horizontal grid with 16 candies at elevation $Y = 6.15$.
   - **Total:** $84 + 60 + 16 = \mathbf{160}$ active procedural candies.
3. **Cyclic Perimeter Modulo Adjacency Formula:**
   - Unfolds the 4 vertical wall faces into a continuous cyclic ring:
     $$
     \text{idx}_{\text{1D}} = (w \cdot S) + c_{\text{local}} \pmod{4 \cdot S}
     $$

     allowing match-3 chains to cross seamlessly over $90^\circ$ exterior corners.
4. **Modern Analytical Illumination & 5 Confectionery Materials:**
   - Dual-light setup: Primary warm key point light at $(8.0, 16.0, 8.0)$ + secondary cool fill directional light along $(-0.6, 0.8, -0.4)$.
   - Real-time comparison between **Flat**, **Gouraud** (per-vertex), and **Blinn-Phong** (per-fragment) shading models.
   - 5 bespoke material shaders: Glossy Jelly (Fresnel rim glow), Frosted Hard Candy (dual-lobe specular), Sugar-Coated Gumdrop (hash noise sparkles), 24K Gold Sweet (metallic glints), and Artisan Hand-Brushed Glaze.
5. **3D Screen-to-World Raycasting & Picking:**
   - Inverts the View-Projection pipeline from 2D screen pixels to a 3D directed ray $\mathbf{R}(t) = \mathbf{O} + t\mathbf{D}$.
   - Solves quadratic ray-sphere intersection ($\Delta = b^2 - 4ac$) against all 160 candies, picking the closest entry hit ($t > 0$).
   - Candies swap via a 3D parabolic arc hop ($h(s) = 4H \cdot s(1-s)$) along wall normals.
6. **2D Immediate-Mode Vector HUD:**
   - Hardware-accelerated batch vector renderer using an orthographic projection matrix.
   - Procedural 7-segment digital font and stroke glyphs without external font libraries (e.g. FreeType) or image atlases.
   - Strict OpenGL pipeline state isolation (depth test disable, alpha blending).

---

## Interactive Controls & Key Bindings

| Key / Input                 | Action                           | Technical Description                                                                                                                                  |
| :-------------------------- | :------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Left Click**        | **3D Tile Picking & Swap** | Unprojects 2D screen click to 3D world ray, solves ray-sphere quadratic intersection, and executes 3D parabolic hop.                                   |
| **Right Drag**        | **Manual 360 Orbit**       | Smooth spherical coordinate camera rotation ($\theta, \phi$) centered at pyramid $(0, 3.2, 0)$.                                                    |
| **Scroll Wheel**      | **Zoom In / Out**          | Adjusts camera spherical orbital radius$r$ with smooth clamping.                                                                                     |
| **`Spacebar`**      | **Auto-Orbit Toggle**      | Pauses or resumes smooth$360^\circ$ continuous cinematic camera rotation.                                                                            |
| **`G` Key**         | **Shading Model Cycle**    | Cycles real-time lighting:**Flat** $\rightarrow$ **Gouraud** (per-vertex) $\rightarrow$ **Blinn-Phong** (per-fragment specular). |
| **`N` Key**         | **Normal Map Visualizer**  | Renders surface normals directly in RGB color space ($\vec{N} \times 0.5 + 0.5$) for academic inspection.                                            |
| **`W` Key**         | **Wireframe Mode**         | Toggles between**Solid Shading** (`GL_FILL`) and **Pure Wireframe** (`GL_LINE`).                                                       |
| **`B` Key**         | **Theme Cycle**            | Cycles board and background palette:**Frosted Marble** $\rightarrow$ **Chocolate Bakery** $\rightarrow$ **Midnight Onyx**.       |
| **`C` Key**         | **Artisan Glaze Toggle**   | Toggles procedural pulled-sugar satin ribbon and bakery glaze swipes on confectionery surfaces.                                                        |
| **`E` / `X` Key** | **Exploded Tier View**     | Vertically separates pyramid tiers along$+Y$ via Hermite smoothstep easing ($Y_{\text{offset}} = \lambda \cdot 3.25 \cdot s(t)$).                  |
| **`P` Key**         | **Projection Switch**      | Toggles between**Perspective ($45^\circ$ FOV)** and **Orthographic Blueprint** camera projections.                                       |
| **`M` Key**         | **4x4 MVP Matrix Dump**    | Dumps live Model, View, Projection, and combined MVP matrices to the terminal for debugging.                                                           |
| **`R` Key**         | **Reshuffle / Reset**      | Re-randomizes the board with the fair-start de-duplication loop and resets score counters.                                                             |
| **`Esc` Key**       | **Quit**                   | Closes the GLFW window and cleans up all GPU resources via RAII.                                                                                       |

---

## The Four Procedural Confectionery Shapes

| Shape                              | Triangle Count | Mathematical Synthesis Principle                                                                                                                                              |
| :--------------------------------- | :------------: | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Red Cherry Bent Cylinder** |     1,560     | Moving Frenet-Serret frame ($\mathbf{T}, \mathbf{N}, \mathbf{B}$) along quadratic spine curve $C(t)$, swept circular cross-section, and dual hemispherical terminal caps. |
| **Blue Flying Saucer**       |     1,152     | Oblate spheroid cinched at equator ($\phi = \pi/2$) with a Gaussian bell-curve flange $R(\phi)$ and exact analytical normal cross-products.                               |
| **Lime Square Pillow**       |     1,120     | Fractional-exponent superquadric equations ($s_1 = s_2 = 0.45$) interpolating between a sphere and a cube with analytical normals.                                          |
| **Orange Water Drop**        |     1,080     | Asymmetric teardrop trigonometric profile pinching to a sharp apex point at$u = 0$ and expanding to a rounded bulb at $u = \pi$.                                          |
| **24K Gold Sweet**           |    Variable    | Rare (5% probability) super-glossy sweet with high specularity ($k_s = 0.95, \alpha = 128$) and double-axis ascending spiral pop physics.                                   |

---

## Project Directory Organization

```
candy_pyramid/
├── Makefile                          # Fast parallel GNU Make build script (`make run`)
├── CMakeLists.txt                    # Standard CMake build configuration
├── build.sh                          # Direct g++ compilation script
├── README.md                         # Project documentation and user guide
├── candy_pyramid                     # Compiled binary executable
│
├── include/                          # Header declarations
│   ├── Mesh.h                        # RAII GPU buffer encapsulation (VAO, VBO, EBO)
│   ├── Primitives.h                  # Procedural geometry math API prototypes
│   ├── Shader.h                      # GLSL shader program manager and fallback compiler
│   ├── Camera.h                      # Spherical LookAt orbital camera and ray unprojection
│   ├── Shape.h                       # ShapeTile struct, animation curves, and model matrix
│   ├── Board.h                       # 3D colosseum topology, picking, FSM, and cascades
│   └── HUD.h                         # 2D batch vertex pipeline and vector stroke fonts
│
├── src/                              # C++ implementation translation units
│   ├── Mesh.cpp                      # Buffer allocation, std::exchange move semantics, drawing
│   ├── Primitives.cpp                # Parametric triangulation math for all 3D geometries
│   ├── Shader.cpp                    # Shader compilation logs and uniform updates
│   ├── Camera.cpp                    # View matrix computation and spherical coordinate damping
│   ├── Shape.cpp                     # Squash-stretch pop physics and bespoke idle dynamics
│   ├── Board.cpp                     # 3D ray-sphere picking, 4-way wall adjacency, wall gravity
│   ├── HUD.cpp                       # Procedural 7-segment font and 2D vector batch rendering
│   └── main.cpp                      # GLFW event loop, keyboard/mouse callbacks, 60 FPS pacing
│
└── shaders/                          # Programmable GLSL shader source files
    ├── wireframe.vert                # 3D Vertex shader: MVP pipeline, per-vertex Gouraud
    ├── wireframe.frag                # 3D Fragment shader: Blinn-Phong, Fresnel, sparkles
    ├── hud.vert                      # 2D Orthographic vertex shader for UI elements
    └── hud.frag                      # 2D Flat color fragment shader
```

---

## Comprehensive Engineering Guide & Viva Documentation

The `notes/` directory contains complete academic documentation written for oral viva defense and examination:

- **[Candy_Pyramid_Engineering_Guide.pdf](notes/Candy_Pyramid_Engineering_Guide.pdf):**
  A **35-page textbook-grade handbook** containing:
  - Plain-English introductions and real-world analogies for every graphics concept.
  - Complete "Variables Explained" tables defining every variable in every mathematical equation.
  - Mathematical proof for why surface normals require the inverse-transpose matrix $(\mathbf{M}^{-1})^T$.
  - Word-for-word spoken scripts for the 6 most common faculty examination viva questions.
  - 5 instant 1-line parameter modifications to demonstrate live code authorship.

---

## Build & Run Instructions

### Dependencies

Ensure the following development packages are installed (Fedora / RHEL):

```bash
sudo dnf install -y gcc-c++ make cmake glfw-devel glew-devel glm-devel mesa-libGL-devel
```

*(On Ubuntu / Debian: `sudo apt install -y build-essential cmake libglfw3-dev libglew-dev libglm-dev libgl1-mesa-dev`)*

### 1. GNU Make (Recommended)

```bash
# Build and run immediately
make run

# Or compile with parallel cores
make -j$(nproc)
./candy_pyramid
```

### 2. CMake Build

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
./candy_pyramid
```

### 3. Direct Shell Script

```bash
chmod +x build.sh
./build.sh
```
