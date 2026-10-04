# 📐 3D Geometric Shape-Match (Pure Wireframe Edition) — Complete Codebase

This document contains the complete project architecture, directory structure, and full source code for all header files, implementation files, shaders, and build scripts for the **3D Geometric Shape-Match Wireframe** project in `candy_pyramid/`.

---

## 📋 Table of Contents

1. [Build Configurations](#build-configurations)
   - [`CMakeLists.txt`](#cmakeliststxt)
   - [`Makefile`](#makefile)
   - [`build.sh`](#buildsh)
2. [GLSL Shaders](#glsl-shaders)
   - [`shaders/wireframe.vert`](#shaderswireframevert)
   - [`shaders/wireframe.frag`](#shaderswireframefrag)
3. [Header Files (include/)](#header-files-include)
   - [`include/Mesh.h`](#includemeshh)
   - [`include/Primitives.h`](#includeprimitivesh)
   - [`include/Shader.h`](#includeshaderh)
   - [`include/Camera.h`](#includecamerah)
   - [`include/Shape.h`](#includeshapeh)
   - [`include/Board.h`](#includeboardh)
4. [Implementation Files (src/)](#implementation-files-src)
   - [`src/Mesh.cpp`](#srcmeshcpp)
   - [`src/Primitives.cpp`](#srcprimitivescpp)
   - [`src/Shader.cpp`](#srcshadercpp)
   - [`src/Camera.cpp`](#srccameracpp)
   - [`src/Shape.cpp`](#srcshapecpp)
   - [`src/Board.cpp`](#srcboardcpp)
   - [`src/main.cpp`](#srcmaincpp)

---

## 🎯 Project Overview & Graphics Architecture

* **Pure Mini-Triangles (GL_TRIANGLES):** Every surface—including cylinder caps, quad walls, and sphere rings—is explicitly triangulated and drawn with `glDrawElements(GL_TRIANGLES, ...)`.
* **Standard Stride (6 Floats = 24 Bytes):**
  $$\text{Stride} = [x, y, z, \quad n_x, n_y, n_z]$$
  - `location = 0`: 3D Vertex Position $(x, y, z)$
  - `location = 1`: Surface Normal Vector $(n_x, n_y, n_z)$
* **Pure Monochrome Wireframe (No 2D Colors):**
  - Trays rendered in soft blueprint slate wireframe `(0.38, 0.42, 0.52)` forming an architectural grid.
  - Shapes rendered in crisp, brilliant white wireframe `(0.95, 0.95, 0.95)`.
  - Selected shape hovers and gently pulses in warm golden wireframe `(1.0, 0.95, 0.40)`.
* **Pure Geometric Shape-Matching:** Matches 3 identical **3D Shapes** (3 Spheres, 3 Cylinders, 3 Cubes, or 3 Diamonds) in a row, demonstrating interaction with 3D geometry rather than 2D colors!
* **3D Screen Raycasting:** Mouse clicks are unprojected into world-space rays via inverted view-projection matrices to pick and swap shapes.
* **3D Arc-Hop Trajectories:** Swapping shapes leap over each other with a parabolic trajectory.
* **Exploded Layer View (`E` key):** Smoothly lifts the 3 tiers apart along the Y-axis so the professor can inspect internal triangle geometry.
* **Solid Architectural Clay (`W` key):** Toggles from wireframe into clean white chalk/clay shading without rainbow colors.

---

## 📁 Directory Tree

```
candy_pyramid/
├── CMakeLists.txt              # Standard CMake build script
├── Makefile                    # One-command build (make / make run)
├── build.sh                    # Direct executable compilation script
├── CODEBASE.md                 # Complete monolithic codebase documentation
├── README.md                   # Project overview & submission guide
│
├── include/                    # Header files
│   ├── Mesh.h                  # GPU buffer wrapper (VAO, VBO, EBO)
│   ├── Primitives.h            # Procedural math generators (Cube, Cylinder, Sphere, Diamond)
│   ├── Shader.h                # GLSL shader program loader with embedded fallback
│   ├── Camera.h                # 360 degree orbital camera with 3D raycasting
│   ├── Candy.h                 # Monochrome ShapeTile struct and animation definitions
│   └── Board.h                 # Board layout, raycast picking, and shape match logic
│
├── src/                        # Implementation files
│   ├── main.cpp                # Main window setup, input callbacks, and render loop
│   ├── Mesh.cpp                # Buffer uploads, drawing, and RAII cleanup
│   ├── Primitives.cpp          # Exact parametric triangulation for all 4 shapes
│   ├── Shader.cpp              # Shader compilation, linking, and uniform setters
│   ├── Camera.cpp              # View matrix and 3D screen ray unprojection
│   ├── Candy.cpp               # ShapeTile 3D arc-hop and wireframe rendering
│   └── Board.cpp               # 3D raycast picking, shape match detection, and gravity drop
│
├── shaders/                    # GLSL Shaders
│   ├── wireframe.vert          # Vertex shader (pos, normal, MVP transformations)
│   └── wireframe.frag          # Fragment shader (wireframe outline & monochrome clay)
│
└── single_file/                # Monolithic fallback
    └── shapes.cpp              # Standalone single-file reference implementation
```

---

## 📦 Build Configurations

<a id="cmakeliststxt"></a>
### `CMakeLists.txt`
**Description:** Standard CMake cross-platform build script

```cmake
cmake_minimum_required(VERSION 3.20)
project(candy_pyramid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# GLM is header-only; find package or rely on system include
find_package(glm QUIET)

add_executable(candy_pyramid
    src/main.cpp
    src/Mesh.cpp
    src/Primitives.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Shape.cpp
    src/Board.cpp
)

target_include_directories(candy_pyramid PRIVATE include)

if (TARGET glm::glm)
    target_link_libraries(candy_pyramid PRIVATE glfw GLEW::GLEW OpenGL::GL glm::glm)
else()
    target_link_libraries(candy_pyramid PRIVATE glfw GLEW::GLEW OpenGL::GL)
endif()

# Copy shaders folder next to binary
add_custom_command(TARGET candy_pyramid POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:candy_pyramid>/shaders)

```

---

<a id="makefile"></a>
### `Makefile`
**Description:** Fast one-command build (make / make run)

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lglfw -lGLEW -lGL -lGLU

SRCS = src/main.cpp src/Mesh.cpp src/Primitives.cpp src/Shader.cpp src/Camera.cpp src/Shape.cpp src/Board.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = candy_pyramid

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean

```

---

<a id="buildsh"></a>
### `build.sh`
**Description:** Direct g++ compilation script for Linux

```bash
#!/usr/bin/env bash
set -e

echo "Compiling 3D Geometric Shape-Match..."
g++ -std=c++17 -O2 \
    -Iinclude \
    src/main.cpp \
    src/Mesh.cpp \
    src/Primitives.cpp \
    src/Shader.cpp \
    src/Camera.cpp \
    src/Shape.cpp \
    src/Board.cpp \
    -o candy_pyramid \
    -lglfw -lGLEW -lGL -lGLU

echo "Compilation successful."
echo "Launching candy_pyramid..."
./candy_pyramid

```

---

## 📦 GLSL Shaders

<a id="shaderswireframevert"></a>
### `shaders/wireframe.vert`
**Description:** Vertex shader handling vertex position and normal transformations

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Transform normal using normal matrix (transpose of inverse)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

```

---

<a id="shaderswireframefrag"></a>
### `shaders/wireframe.frag`
**Description:** Fragment shader for high-contrast wireframe lines and architectural clay

```glsl
#version 330 core
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;
uniform bool isTray;

void main() {
    if (isWireframe) {
        if (isTray) {
            // Muted slate blueprint wireframe for pyramid slabs
            FragColor = vec4(0.38, 0.42, 0.52, 1.0);
        } else {
            // Crisp high-contrast white wireframe for geometric shapes
            FragColor = objectColor;
        }
    } else {
        // 100% flat unshaded (zero lighting math)
        if (isTray) {
            FragColor = vec4(0.40, 0.44, 0.52, 1.0); // Flat slate for slabs
        } else {
            FragColor = vec4(0.85, 0.85, 0.85, 1.0); // 100% flat unshaded
        }
    }
}

```

---

## 📦 Header Files (include/)

<a id="includemeshh"></a>
### `include/Mesh.h`
**Description:** GPU buffer wrapper (VAO, VBO, EBO) for pure GL_TRIANGLES

```cpp
#pragma once
#include <GL/glew.h>
#include <vector>

// Struct to store GPU buffers and render state
// Uses pure GL_TRIANGLES for all geometric meshes
class Mesh {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    unsigned int indexCount = 0;

    Mesh() = default;
    ~Mesh();

    // Disable copy semantics to prevent OpenGL handle double-deletion
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Enable move semantics
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Upload interleaved vertex data [x, y, z,  nx, ny, nz] and element indices
    void setupBuffers(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    // Draw the mesh with pure GL_TRIANGLES
    void draw() const;

    // Free GPU buffers
    void destroy();
};

```

---

<a id="includeprimitivesh"></a>
### `include/Primitives.h`
**Description:** Procedural shape math generators (Cube, Cylinder, Sphere, Diamond)

```cpp
#pragma once
#include "Mesh.h"

namespace Primitives {

/**
 * 1. CUBE GENERATOR (8 Vertices -> 24 Vertices with Flat Normals -> 12 Triangles per Box)
 * Used for:
 *   - Stepped Pyramid Slabs (Layer 0, 1, 2 Trays)
 *   - Square Chiclet Candies
 */
Mesh createCube();

/**
 * 2. CYLINDER / PRISM GENERATOR (Pure Triangles: Side Quads -> 2 Tris + Fan Caps)
 * Used for:
 *   - Flat Hexagonal (sectors=6) / Octagonal (sectors=8) Lozenge Candies
 *   - Cylindrical Gumdrop Candies
 * 
 * @param sectors Number of angular segments around the circumference
 * @param radius  Radius of the cylinder
 * @param height  Total vertical height of the cylinder
 */
Mesh createCylinder(int sectors = 8, float radius = 0.4f, float height = 0.3f);

/**
 * 3. UV SPHERE GENERATOR (Latitude / Longitude Triangulation)
 * Used for:
 *   - Spherical Lemon Drop / Ball Candies
 * 
 * @param rings   Number of horizontal latitude divisions (0 to PI)
 * @param sectors Number of vertical longitude divisions (0 to 2*PI)
 * @param radius  Radius of the sphere
 */
Mesh createSphere(int rings = 16, int sectors = 20, float radius = 0.4f);

/**
 * 4. DIAMOND / BIPYRAMID GENERATOR (Double-cone Triangulation)
 * Used for:
 *   - Faceted Gem / Diamond Candies
 * 
 * @param facets Number of equatorial facets
 * @param radius Radius of the equatorial ring
 * @param height Total tip-to-tip vertical height
 */
Mesh createDiamond(int facets = 8, float radius = 0.4f, float height = 0.6f);

} // namespace Primitives

```

---

<a id="includeshaderh"></a>
### `include/Shader.h`
**Description:** GLSL shader program compiler and uniform manager with fallback

```cpp
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint programId = 0;

    Shader() = default;
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Move semantics
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Load from disk with fallback to embedded defaults if file is missing
    bool init(const std::string& vertexPath, const std::string& fragmentPath);
    bool initFromSource(const char* vertexCode, const char* fragmentCode);

    void use() const;
    void destroy();

    // Uniform setters
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setFloat(const std::string& name, float val) const;
    void setInt(const std::string& name, int val) const;
    void setBool(const std::string& name, bool val) const;

private:
    static GLuint compileShader(GLenum type, const char* source);
};

```

---

<a id="includecamerah"></a>
### `include/Camera.h`
**Description:** 360 degree orbital camera with 3D raycasting unprojection

```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

class Camera {
public:
    glm::vec3 target = glm::vec3(0.0f, 1.2f, 0.0f);
    float distance = 14.5f;
    float yaw = 45.0f;       // Horizontal orbit angle (degrees)
    float pitch = 32.0f;     // Vertical elevation angle (degrees)
    bool autoOrbit = true;   // Automatically orbit around the pyramid
    float orbitSpeed = 16.0f;// Degrees per second

    Camera() = default;

    void update(float deltaTime);
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;

    void processMouseDrag(float xoffset, float yoffset);
    void processScroll(float yoffset);
    void reset();

    // 3D Raycasting from mouse screen coordinates
    Ray getRayFromScreen(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projection) const;
};


```

---

<a id="includeshapeh"></a>
### `include/Shape.h`
**Description:** Monochrome geometric shape tile with 3D arc-hop & wireframe animations

```cpp
#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Mesh.h"
#include "Shader.h"

enum class ShapeType {
    NONE = -1,
    SPHERE = 0,    // UV Sphere (640 tris)
    CYLINDER = 1,  // Octagonal Prism (32 tris)
    CUBE = 2,      // Unit Cube (12 tris)
    DIAMOND = 3,   // Double-Cone Bipyramid (16 tris)
    COUNT = 4
};

std::string getShapeName(ShapeType type);

struct ShapeTile {
    ShapeType type = ShapeType::NONE;
    int layer = 0;
    int row = 0;
    int col = 0;

    // 3D coordinates
    glm::vec3 currentPos = glm::vec3(0.0f);
    glm::vec3 targetPos  = glm::vec3(0.0f);
    float scale = 1.0f;
    float rotationY = 0.0f;

    // Animation status
    bool isAnimating = false;
    bool isPopping = false;
    float animProgress = 1.0f;
    float animDuration = 0.22f;
    glm::vec3 startPos = glm::vec3(0.0f);
    float jumpHeight = 0.0f;

    void startMove(const glm::vec3& to, float duration = 0.22f, float arcHeight = 0.0f);
    void startPop(float duration = 0.22f);
    void update(float deltaTime);
    glm::mat4 getModelMatrix(float time = 0.0f, bool isSelected = false) const;

    void draw(const Shader& shader, const Mesh& sphereMesh, const Mesh& cylinderMesh,
              const Mesh& cubeMesh, const Mesh& diamondMesh, bool isSelected, float time) const;
};


```

---

<a id="includeboardh"></a>
### `include/Board.h`
**Description:** Stepped pyramid board state, raycast picking, and shape match-3 logic

```cpp
#pragma once
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include "Shape.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

struct BoardCoord {
    int layer = -1;
    int row = -1;
    int col = -1;

    bool operator==(const BoardCoord& o) const {
        return layer == o.layer && row == o.row && col == o.col;
    }
    bool operator!=(const BoardCoord& o) const {
        return !(*this == o);
    }
    bool operator<(const BoardCoord& o) const {
        if (layer != o.layer) return layer < o.layer;
        if (row != o.row) return row < o.row;
        return col < o.col;
    }
    bool isValid() const {
        return layer >= 0;
    }
};

enum class GameState {
    IDLE,
    SWAPPING,
    CHECKING_MATCH,
    POPPING,
    FALLING,
    REVERTING
};

class Board {
public:
    int score = 0;
    int combo = 0;
    GameState state = GameState::IDLE;

    BoardCoord selectedCoord;
    BoardCoord swapTargetCoord;

    bool isExploded = false;
    float explodeProgress = 0.0f;

    Board();

    void reset();
    void update(float deltaTime);
    void toggleExplode();

    // 3D Raycasting selection
    BoardCoord pickTile(const Ray& ray) const;
    void onTileClicked(const BoardCoord& coord);

    // Draw full scene (pyramid trays + shapes in pure wireframe)
    void draw(const Shader& shader, const Mesh& cubeMesh, const Mesh& sphereMesh,
              const Mesh& cylinderMesh, const Mesh& diamondMesh, float time) const;

    ShapeTile* getTile(const BoardCoord& coord);
    const ShapeTile* getTile(const BoardCoord& coord) const;
    bool isValidCoord(const BoardCoord& coord) const;
    bool isAdjacent(const BoardCoord& a, const BoardCoord& b) const;
    glm::vec3 getSlotWorldPos(int layer, int row, int col) const;

private:
    std::vector<ShapeTile> layer2; // 4x4  = 16 tiles (Top)
    std::vector<ShapeTile> layer1; // 6x6  = 36 tiles (Middle perimeter)
    std::vector<ShapeTile> layer0; // 8x8  = 64 tiles (Base perimeter)

    float stateTimer = 0.0f;
    std::set<BoardCoord> matchedCoords;

    void initLayerShapes();
    bool checkAndMarkMatches();
    void applyGravityAndRefill();
    void executeSwap(const BoardCoord& a, const BoardCoord& b, bool isRevert = false);
    ShapeType getRandomShapeType() const;
};

```

---

## 📦 Implementation Files (src/)

<a id="srcmeshcpp"></a>
### `src/Mesh.cpp`
**Description:** Buffer allocation, stride setup, and indexed rendering

```cpp
#include "Mesh.h"
#include <utility>

Mesh::~Mesh() {
    destroy();
}

Mesh::Mesh(Mesh&& other) noexcept 
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        destroy();
        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        indexCount = other.indexCount;

        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh::setupBuffers(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    destroy(); // Clean existing if re-uploading

    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Upload interleaved vertex data [x, y, z,  nx, ny, nz]
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Upload indexed triangle elements (GL_TRIANGLES)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Location 0: Vertex Position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Location 1: Surface Normal Vector (3 floats, ready for Day 2 Blinn-Phong lighting)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (vao != 0 && indexCount > 0) {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Mesh::destroy() {
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    indexCount = 0;
}

```

---

<a id="srcprimitivescpp"></a>
### `src/Primitives.cpp`
**Description:** Parametric triangulation math for all 4 procedural shapes

```cpp
#include "Primitives.h"
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Primitives {

// -----------------------------------------------------------------------------
// 1. CUBE GENERATOR (24 Vertices, 12 Triangles per Box)
// Stride = 6 floats: [posX, posY, posZ,  normX, normY, normZ]
// -----------------------------------------------------------------------------
Mesh createCube() {
    Mesh mesh;

    // 24 vertices (4 per face to maintain flat, sharp face normals)
    std::vector<float> vertices = {
        // Front face (Z = +0.5) - Normal: (0, 0, 1)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // Back face (Z = -0.5) - Normal: (0, 0, -1)
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        // Top face (Y = +0.5) - Normal: (0, 1, 0)
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,

        // Bottom face (Y = -0.5) - Normal: (0, -1, 0)
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,

        // Right face (X = +0.5) - Normal: (1, 0, 0)
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

        // Left face (X = -0.5) - Normal: (-1, 0, 0)
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f
    };

    // 12 Triangles (2 per face) -> Pure GL_TRIANGLES
    std::vector<unsigned int> indices = {
         0,  1,  2,   0,  2,  3, // Front
         4,  5,  6,   4,  6,  7, // Back
         8,  9, 10,   8, 10, 11, // Top
        12, 13, 14,  12, 14, 15, // Bottom
        16, 17, 18,  16, 18, 19, // Right
        20, 21, 22,  20, 22, 23  // Left
    };

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 2. CYLINDER / PRISM GENERATOR (Pure Triangles: Side Quads -> 2 Tris + Fan Caps)
// -----------------------------------------------------------------------------
Mesh createCylinder(int sectors, float radius, float height) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfH = height / 2.0f;
    float pi2 = 2.0f * static_cast<float>(M_PI);

    // --- Side Body (Radial quad strips) ---
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * pi2 / static_cast<float>(sectors);
        float x = std::cos(angle);
        float z = std::sin(angle);

        // Top rim vertex [x, y, z,  nx, ny, nz]
        vertices.push_back(radius * x);
        vertices.push_back(halfH);
        vertices.push_back(radius * z);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Bottom rim vertex [x, y, z,  nx, ny, nz]
        vertices.push_back(radius * x);
        vertices.push_back(-halfH);
        vertices.push_back(radius * z);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    for (int i = 0; i < sectors; ++i) {
        unsigned int i0 = i * 2;
        unsigned int i1 = i0 + 1;
        unsigned int i2 = i0 + 2;
        unsigned int i3 = i0 + 3;

        // Two non-overlapping triangles per quad sector
        indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
        indices.push_back(i2); indices.push_back(i1); indices.push_back(i3);
    }

    // --- Top Cap (Triangulated from central apex) ---
    unsigned int topCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    vertices.push_back(0.0f);
    vertices.push_back(halfH);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    unsigned int topRimStart = static_cast<unsigned int>(vertices.size() / 6);
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * pi2 / static_cast<float>(sectors);
        vertices.push_back(radius * std::cos(angle));
        vertices.push_back(halfH);
        vertices.push_back(radius * std::sin(angle));
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(topCenterIdx);
        indices.push_back(topRimStart + i);
        indices.push_back(topRimStart + i + 1);
    }

    // --- Bottom Cap (Triangulated from central apex) ---
    unsigned int botCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    vertices.push_back(0.0f);
    vertices.push_back(-halfH);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);

    unsigned int botRimStart = static_cast<unsigned int>(vertices.size() / 6);
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * pi2 / static_cast<float>(sectors);
        vertices.push_back(radius * std::cos(angle));
        vertices.push_back(-halfH);
        vertices.push_back(radius * std::sin(angle));
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
    }
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(botCenterIdx);
        indices.push_back(botRimStart + i + 1);
        indices.push_back(botRimStart + i);
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 3. UV SPHERE GENERATOR (Latitude / Longitude Triangulation)
// -----------------------------------------------------------------------------
Mesh createSphere(int rings, int sectors, float radius) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float pi = static_cast<float>(M_PI);
    float pi2 = 2.0f * pi;

    for (int r = 0; r <= rings; ++r) {
        float phi = pi * static_cast<float>(r) / static_cast<float>(rings); // 0 (North Pole) to PI (South Pole)
        for (int s = 0; s <= sectors; ++s) {
            float theta = pi2 * static_cast<float>(s) / static_cast<float>(sectors); // 0 to 2PI

            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);

            // Position [x*r, y*r, z*r] & Unit Normal [x, y, z]
            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate two non-overlapping triangles per quad element
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first = static_cast<unsigned int>((r * (sectors + 1)) + s);
            unsigned int second = first + static_cast<unsigned int>(sectors + 1);

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 4. DIAMOND / BIPYRAMID GENERATOR (Double-cone Triangulation)
// -----------------------------------------------------------------------------
Mesh createDiamond(int facets, float radius, float height) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfH = height / 2.0f;
    float pi2 = 2.0f * static_cast<float>(M_PI);

    // Top Apex
    unsigned int topApexIdx = 0;
    vertices.push_back(0.0f); vertices.push_back(halfH); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(1.0f);  vertices.push_back(0.0f);

    // Bottom Apex
    unsigned int botApexIdx = 1;
    vertices.push_back(0.0f); vertices.push_back(-halfH); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(-1.0f);  vertices.push_back(0.0f);

    // Equatorial Ring Vertices
    unsigned int ringStart = 2;
    for (int i = 0; i <= facets; ++i) {
        float angle = i * pi2 / static_cast<float>(facets);
        float x = std::cos(angle);
        float z = std::sin(angle);

        vertices.push_back(radius * x);
        vertices.push_back(0.0f);
        vertices.push_back(radius * z);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    for (int i = 0; i < facets; ++i) {
        unsigned int curr = ringStart + i;
        unsigned int next = ringStart + i + 1;

        // Top triangle fan
        indices.push_back(topApexIdx);
        indices.push_back(curr);
        indices.push_back(next);

        // Bottom triangle fan
        indices.push_back(botApexIdx);
        indices.push_back(next);
        indices.push_back(curr);
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

} // namespace Primitives

```

---

<a id="srcshadercpp"></a>
### `src/Shader.cpp`
**Description:** Shader file loading, compilation checks, and uniform binding

```cpp
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static const char* DEFAULT_VERT = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char* DEFAULT_FRAG = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;

void main() {
    if (isWireframe) {
        // High-contrast clean outline for wireframe inspection
        FragColor = vec4(0.95, 0.95, 0.95, 1.0);
    } else {
        // Simple directional diffuse so solid mode showcases 3D depth
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 ambient = 0.35 * objectColor.rgb;
        vec3 diffuse = diff * 0.65 * objectColor.rgb;
        FragColor = vec4(ambient + diffuse, objectColor.a);
    }
}
)";

Shader::~Shader() {
    destroy();
}

Shader::Shader(Shader&& other) noexcept : programId(other.programId) {
    other.programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        destroy();
        programId = other.programId;
        other.programId = 0;
    }
    return *this;
}

bool Shader::init(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vCode, fCode;
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    if (vFile.is_open() && fFile.is_open()) {
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        vCode = vStream.str();
        fCode = fStream.str();
        return initFromSource(vCode.c_str(), fCode.c_str());
    } else {
        std::cout << "[Shader] Using embedded default shaders (could not find external file)." << std::endl;
        return initFromSource(DEFAULT_VERT, DEFAULT_FRAG);
    }
}

bool Shader::initFromSource(const char* vertexCode, const char* fragmentCode) {
    destroy();

    GLuint vShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    if (!vShader) return false;

    GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);
    if (!fShader) {
        glDeleteShader(vShader);
        return false;
    }

    programId = glCreateProgram();
    glAttachShader(programId, vShader);
    glAttachShader(programId, fShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        std::cerr << "[Shader Link Error]: " << infoLog << std::endl;
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        destroy();
        return false;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return true;
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "[Shader Compile Error (" << (type == GL_VERTEX_SHADER ? "VERT" : "FRAG") << ")]: "
                  << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::use() const {
    if (programId != 0) {
        glUseProgram(programId);
    }
}

void Shader::destroy() {
    if (programId != 0) {
        glDeleteProgram(programId);
        programId = 0;
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec4(const std::string& name, const glm::vec4& vec) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string& name, float val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1f(loc, val);
}

void Shader::setInt(const std::string& name, int val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1i(loc, val);
}

void Shader::setBool(const std::string& name, bool val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1i(loc, val ? 1 : 0);
}

```

---

<a id="srccameracpp"></a>
### `src/Camera.cpp`
**Description:** Spherical orbital trigonometry and 3D screen ray unprojection

```cpp
#include "Camera.h"
#include <cmath>
#include <algorithm>

void Camera::update(float deltaTime) {
    if (autoOrbit) {
        yaw += orbitSpeed * deltaTime;
        if (yaw >= 360.0f) yaw -= 360.0f;
    }
}

glm::vec3 Camera::getPosition() const {
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    float x = target.x + distance * std::cos(pitchRad) * std::sin(yawRad);
    float y = target.y + distance * std::sin(pitchRad);
    float z = target.z + distance * std::cos(pitchRad) * std::cos(yawRad);

    return glm::vec3(x, y, z);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::processMouseDrag(float xoffset, float yoffset) {
    float sensitivity = 0.25f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    // Clamp pitch to prevent camera flip
    pitch = std::clamp(pitch, -85.0f, 85.0f);

    if (yaw >= 360.0f) yaw -= 360.0f;
    if (yaw < 0.0f) yaw += 360.0f;
}

void Camera::processScroll(float yoffset) {
    distance -= yoffset * 0.8f;
    distance = std::clamp(distance, 4.0f, 35.0f);
}

void Camera::reset() {
    target = glm::vec3(0.0f, 1.2f, 0.0f);
    distance = 14.5f;
    yaw = 45.0f;
    pitch = 32.0f;
    autoOrbit = true;
}

Ray Camera::getRayFromScreen(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projection) const {
    float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(screenHeight);

    glm::mat4 invVP = glm::inverse(projection * getViewMatrix());
    glm::vec4 nearPoint = invVP * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farPoint  = invVP * glm::vec4(x, y,  1.0f, 1.0f);

    nearPoint /= nearPoint.w;
    farPoint  /= farPoint.w;

    Ray ray;
    ray.origin = glm::vec3(nearPoint);
    ray.direction = glm::normalize(glm::vec3(farPoint - nearPoint));
    return ray;
}


```

---

<a id="srcshapecpp"></a>
### `src/Shape.cpp`
**Description:** ShapeTile animation interpolation and pure wireframe drawing

```cpp
#include "Shape.h"
#include <string>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::string getShapeName(ShapeType type) {
    switch (type) {
        case ShapeType::SPHERE:   return "Sphere (UV Lat/Long)";
        case ShapeType::CYLINDER: return "Cylinder (Octagonal)";
        case ShapeType::CUBE:     return "Cube (Chiclet)";
        case ShapeType::DIAMOND:  return "Diamond (Bipyramid)";
        default:                  return "Empty";
    }
}

void ShapeTile::startMove(const glm::vec3& to, float duration, float arcHeight) {
    isAnimating = true;
    startPos = currentPos;
    targetPos = to;
    animDuration = duration;
    animProgress = 0.0f;
    jumpHeight = arcHeight;
}

void ShapeTile::startPop(float duration) {
    isPopping = true;
    isAnimating = true;
    animDuration = duration;
    animProgress = 0.0f;
}

void ShapeTile::update(float deltaTime) {
    if (!isAnimating) return;

    animProgress += deltaTime / animDuration;
    if (animProgress >= 1.0f) {
        animProgress = 1.0f;
        isAnimating = false;
        currentPos = targetPos;
        if (isPopping) {
            scale = 0.0f;
            type = ShapeType::NONE;
            isPopping = false;
        }
    } else {
        float t = animProgress;
        if (isPopping) {
            // Shrink and spin during wireframe pop
            scale = 1.0f - t;
            rotationY += deltaTime * 12.0f;
        } else {
            // Smooth step 3D arc hop
            float smoothT = t * t * (3.0f - 2.0f * t);
            currentPos.x = glm::mix(startPos.x, targetPos.x, smoothT);
            currentPos.z = glm::mix(startPos.z, targetPos.z, smoothT);
            float baseY  = glm::mix(startPos.y, targetPos.y, smoothT);
            currentPos.y = baseY + jumpHeight * std::sin(static_cast<float>(M_PI) * t);
        }
    }
}

glm::mat4 ShapeTile::getModelMatrix(float time, bool isSelected) const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), currentPos);

    if (isSelected) {
        // Selected tile bounces up and rotates smoothly
        float hoverY = 0.15f + 0.05f * std::sin(time * 8.0f);
        model = glm::translate(model, glm::vec3(0.0f, hoverY, 0.0f));
        model = glm::rotate(model, time * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (rotationY != 0.0f) {
        model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    switch (type) {
        case ShapeType::SPHERE:
            return glm::scale(model, glm::vec3(scale * 0.95f));
        case ShapeType::CYLINDER:
            return glm::scale(model, glm::vec3(scale * 1.0f, scale * 0.9f, scale * 1.0f));
        case ShapeType::CUBE:
            return glm::scale(model, glm::vec3(scale * 0.65f, scale * 0.45f, scale * 0.65f));
        case ShapeType::DIAMOND:
            return glm::scale(model, glm::vec3(scale * 0.90f, scale * 0.90f, scale * 0.90f));
        default:
            return model;
    }
}

void ShapeTile::draw(const Shader& shader, const Mesh& sphereMesh, const Mesh& cylinderMesh,
                    const Mesh& cubeMesh, const Mesh& diamondMesh, bool isSelected, float time) const {
    if (type == ShapeType::NONE || scale <= 0.001f) return;

    glm::mat4 model = getModelMatrix(time, isSelected);

    if (isSelected) {
        // Subtle warm highlight for selection feedback
        shader.setVec4("objectColor", glm::vec4(1.0f, 0.95f, 0.40f, 1.0f));
    } else {
        // Pure monochrome crisp white wireframe (no color)
        shader.setVec4("objectColor", glm::vec4(0.95f, 0.95f, 0.95f, 1.0f));
    }

    shader.setBool("isTray", false);
    shader.setMat4("model", model);

    // Draw corresponding geometric primitive
    switch (type) {
        case ShapeType::SPHERE:   sphereMesh.draw(); break;
        case ShapeType::CYLINDER: cylinderMesh.draw(); break;
        case ShapeType::CUBE:     cubeMesh.draw(); break;
        case ShapeType::DIAMOND:  diamondMesh.draw(); break;
        default: break;
    }
}

```

---

<a id="srcboardcpp"></a>
### `src/Board.cpp`
**Description:** Board layout, 3D raycast picking, shape match detection, and gravity drop

```cpp
#include "Board.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <iostream>
#include <cmath>

Board::Board() {
    layer2.resize(16);
    layer1.resize(36);
    layer0.resize(64);
    reset();
}

void Board::reset() {
    score = 0;
    combo = 0;
    state = GameState::IDLE;
    selectedCoord = BoardCoord();
    swapTargetCoord = BoardCoord();
    matchedCoords.clear();
    initLayerShapes();
}

ShapeType Board::getRandomShapeType() const {
    return static_cast<ShapeType>(std::rand() % static_cast<int>(ShapeType::COUNT));
}

void Board::toggleExplode() {
    isExploded = !isExploded;
    std::cout << "[View] Exploded Layer Mode " << (isExploded ? "ACTIVATED" : "DEACTIVATED") << std::endl;
    std::cout << "  [Transform: Explode Hierarchy] Tier Y-Offsets:\n"
              << "    Layer 2 (Top 4x4)    : T( 0.000, " << (isExploded ? "+4.950" : "+2.450") << ", 0.000 )\n"
              << "    Layer 1 (Middle 6x6) : T( 0.000, " << (isExploded ? "+2.700" : "+1.450") << ", 0.000 )\n"
              << "    Layer 0 (Base 8x8)   : T( 0.000, +0.450, 0.000 )" << std::endl;
}


glm::vec3 Board::getSlotWorldPos(int layer, int row, int col) const {
    float explodeOffset = 0.0f;
    if (layer == 1) explodeOffset = 1.25f * explodeProgress;
    else if (layer == 2) explodeOffset = 2.50f * explodeProgress;

    switch (layer) {
        case 2:
            return glm::vec3((col - 1.5f) * 0.95f, 2.45f + explodeOffset, (row - 1.5f) * 0.95f);
        case 1:
            return glm::vec3((col - 2.5f) * 0.95f, 1.45f + explodeOffset, (row - 2.5f) * 0.95f);
        case 0:
        default:
            return glm::vec3((col - 3.5f) * 0.95f, 0.45f + explodeOffset, (row - 3.5f) * 0.95f);
    }
}

bool Board::isValidCoord(const BoardCoord& c) const {
    if (!c.isValid()) return false;
    if (c.layer == 2) {
        return (c.row >= 0 && c.row < 4 && c.col >= 0 && c.col < 4);
    } else if (c.layer == 1) {
        if (c.row < 0 || c.row >= 6 || c.col < 0 || c.col >= 6) return false;
        // Outer exposed perimeter of 6x6
        return (c.row == 0 || c.row == 5 || c.col == 0 || c.col == 5);
    } else if (c.layer == 0) {
        if (c.row < 0 || c.row >= 8 || c.col < 0 || c.col >= 8) return false;
        // Outer exposed perimeter of 8x8
        return (c.row == 0 || c.row == 7 || c.col == 0 || c.col == 7);
    }
    return false;
}

ShapeTile* Board::getTile(const BoardCoord& c) {
    if (!isValidCoord(c)) return nullptr;
    if (c.layer == 2) return &layer2[c.row * 4 + c.col];
    if (c.layer == 1) return &layer1[c.row * 6 + c.col];
    if (c.layer == 0) return &layer0[c.row * 8 + c.col];
    return nullptr;
}

const ShapeTile* Board::getTile(const BoardCoord& c) const {
    if (!isValidCoord(c)) return nullptr;
    if (c.layer == 2) return &layer2[c.row * 4 + c.col];
    if (c.layer == 1) return &layer1[c.row * 6 + c.col];
    if (c.layer == 0) return &layer0[c.row * 8 + c.col];
    return nullptr;
}

bool Board::isAdjacent(const BoardCoord& a, const BoardCoord& b) const {
    if (!isValidCoord(a) || !isValidCoord(b)) return false;
    if (a.layer != b.layer) return false; // Swapping is within the same terrace tier

    int dr = std::abs(a.row - b.row);
    int dc = std::abs(a.col - b.col);

    if (a.layer == 2) {
        return (dr + dc == 1);
    }

    // On perimeter rings (layers 1 and 0), check ring-adjacency
    int size = (a.layer == 1) ? 6 : 8;
    auto getPerimeterIndex = [size](int r, int c) -> int {
        if (r == 0) return c;
        if (c == size - 1) return (size - 1) + r;
        if (r == size - 1) return (size - 1) + (size - 1) + ((size - 1) - c);
        if (c == 0) return (size - 1) * 3 + ((size - 1) - r);
        return -1;
    };

    int idxA = getPerimeterIndex(a.row, a.col);
    int idxB = getPerimeterIndex(b.row, b.col);
    if (idxA < 0 || idxB < 0) return false;

    int totalPerim = (size - 1) * 4;
    int diff = std::abs(idxA - idxB);
    return (diff == 1 || diff == totalPerim - 1);
}

void Board::initLayerShapes() {
    // Fill all exposed terrace slots with random shapes avoiding initial 3-matches
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            ShapeTile* t = getTile({2, r, c});
            t->layer = 2; t->row = r; t->col = c;
            t->currentPos = getSlotWorldPos(2, r, c);
            t->targetPos = t->currentPos;
            t->scale = 1.0f;
            t->isAnimating = false;
            t->type = getRandomShapeType();
        }
    }

    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 6; ++c) {
            if (!isValidCoord({1, r, c})) continue;
            ShapeTile* t = getTile({1, r, c});
            t->layer = 1; t->row = r; t->col = c;
            t->currentPos = getSlotWorldPos(1, r, c);
            t->targetPos = t->currentPos;
            t->scale = 1.0f;
            t->isAnimating = false;
            t->type = getRandomShapeType();
        }
    }

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (!isValidCoord({0, r, c})) continue;
            ShapeTile* t = getTile({0, r, c});
            t->layer = 0; t->row = r; t->col = c;
            t->currentPos = getSlotWorldPos(0, r, c);
            t->targetPos = t->currentPos;
            t->scale = 1.0f;
            t->isAnimating = false;
            t->type = getRandomShapeType();
        }
    }

    // Re-roll any accidental initial match-3
    for (int attempts = 0; attempts < 15; ++attempts) {
        if (!checkAndMarkMatches()) break;
        for (const auto& coord : matchedCoords) {
            getTile(coord)->type = getRandomShapeType();
        }
        matchedCoords.clear();
    }
}

BoardCoord Board::pickTile(const Ray& ray) const {
    BoardCoord bestCoord;
    float closestDist = 1e9f;

    auto testLayer = [&](int layer, int rows, int cols) {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                BoardCoord coord{layer, r, c};
                if (!isValidCoord(coord)) continue;
                const ShapeTile* t = getTile(coord);
                if (!t || t->type == ShapeType::NONE) continue;

                // Sphere-ray intersection test
                glm::vec3 oc = ray.origin - t->currentPos;
                float b = glm::dot(oc, ray.direction);
                float cVal = glm::dot(oc, oc) - (0.50f * 0.50f);
                float disc = b * b - cVal;
                if (disc >= 0.0f) {
                    float tHit = -b - std::sqrt(disc);
                    if (tHit > 0.1f && tHit < closestDist) {
                        closestDist = tHit;
                        bestCoord = coord;
                    }
                }
            }
        }
    };

    testLayer(2, 4, 4);
    testLayer(1, 6, 6);
    testLayer(0, 8, 8);

    return bestCoord;
}

void Board::onTileClicked(const BoardCoord& clicked) {
    if (state != GameState::IDLE) return;
    if (!isValidCoord(clicked)) return;

    if (!selectedCoord.isValid()) {
        selectedCoord = clicked;
        const ShapeTile* t = getTile(clicked);
        std::cout << "[Selected] Layer " << clicked.layer << " (" << clicked.row << ", " << clicked.col << ") - "
                  << getShapeName(t ? t->type : ShapeType::NONE) << std::endl;
        if (t) {
            std::cout << "  [Transform: Model] " << getShapeName(t->type) << "\n"
                      << "    Translation : T( " << t->currentPos.x << ", " << t->currentPos.y << ", " << t->currentPos.z << " )\n"
                      << "    Rotation    : R_y( active hover + continuous spin )\n"
                      << "    Scale       : S( " << t->scale << ", " << t->scale << ", " << t->scale << " )" << std::endl;
        }
        return;
    }

    if (selectedCoord == clicked) {
        selectedCoord = BoardCoord(); // Deselect
        std::cout << "[Deselected]" << std::endl;
        return;
    }

    if (isAdjacent(selectedCoord, clicked)) {
        swapTargetCoord = clicked;
        executeSwap(selectedCoord, swapTargetCoord, false);
    } else {
        selectedCoord = clicked;
        const ShapeTile* t = getTile(clicked);
        std::cout << "[Selected] Layer " << clicked.layer << " (" << clicked.row << ", " << clicked.col << ") - "
                  << getShapeName(t ? t->type : ShapeType::NONE) << std::endl;
        if (t) {
            std::cout << "  [Transform: Model] " << getShapeName(t->type) << "\n"
                      << "    Translation : T( " << t->currentPos.x << ", " << t->currentPos.y << ", " << t->currentPos.z << " )\n"
                      << "    Rotation    : R_y( active hover + continuous spin )\n"
                      << "    Scale       : S( " << t->scale << ", " << t->scale << ", " << t->scale << " )" << std::endl;
        }
    }
}

void Board::executeSwap(const BoardCoord& a, const BoardCoord& b, bool isRevert) {
    ShapeTile* tA = getTile(a);
    ShapeTile* tB = getTile(b);
    if (!tA || !tB) return;

    glm::vec3 posA = getSlotWorldPos(a.layer, a.row, a.col);
    glm::vec3 posB = getSlotWorldPos(b.layer, b.row, b.col);

    if (!isRevert) {
        std::cout << "[Transform: Swap Arc] Swapping Layer " << a.layer << " (" << a.row << "," << a.col << ") <-> (" << b.row << "," << b.col << ")\n"
                  << "  Tile A Pos : [ " << posA.x << ", " << posA.y << ", " << posA.z << " ] -> [ " << posB.x << ", " << posB.y << ", " << posB.z << " ]\n"
                  << "  Tile B Pos : [ " << posB.x << ", " << posB.y << ", " << posB.z << " ] -> [ " << posA.x << ", " << posA.y << ", " << posA.z << " ]\n"
                  << "  Trajectory : Y(t) = mix(Y_start, Y_end, t) + 0.45 * sin(pi * t)\n"
                  << "  Duration   : 0.20 sec (SmoothStep interpolation)" << std::endl;
    }

    // 3D Arc-hop trajectories
    tA->startMove(posB, 0.20f, 0.45f);
    tB->startMove(posA, 0.20f, -0.25f);

    state = isRevert ? GameState::REVERTING : GameState::SWAPPING;
    stateTimer = 0.22f;
}


bool Board::checkAndMarkMatches() {
    matchedCoords.clear();

    // 1. Scan Top Layer (4x4 Grid)
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 2; ++c) {
            ShapeType t0 = getTile({2, r, c})->type;
            ShapeType t1 = getTile({2, r, c + 1})->type;
            ShapeType t2 = getTile({2, r, c + 2})->type;
            if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                matchedCoords.insert({2, r, c});
                matchedCoords.insert({2, r, c + 1});
                matchedCoords.insert({2, r, c + 2});
            }
        }
    }
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 2; ++r) {
            ShapeType t0 = getTile({2, r, c})->type;
            ShapeType t1 = getTile({2, r + 1, c})->type;
            ShapeType t2 = getTile({2, r + 2, c})->type;
            if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                matchedCoords.insert({2, r, c});
                matchedCoords.insert({2, r + 1, c});
                matchedCoords.insert({2, r + 2, c});
            }
        }
    }

    // 2. Scan Middle (6x6) and Base (8x8) along continuous perimeter rings
    auto scanPerimeter = [&](int layer, int size) {
        std::vector<BoardCoord> ring;
        for (int c = 0; c < size; ++c) ring.push_back({layer, 0, c});
        for (int r = 1; r < size; ++r) ring.push_back({layer, r, size - 1});
        for (int c = size - 2; c >= 0; --c) ring.push_back({layer, size - 1, c});
        for (int r = size - 2; r >= 1; --r) ring.push_back({layer, r, 0});

        int N = static_cast<int>(ring.size());
        for (int i = 0; i < N; ++i) {
            ShapeType t0 = getTile(ring[i])->type;
            ShapeType t1 = getTile(ring[(i + 1) % N])->type;
            ShapeType t2 = getTile(ring[(i + 2) % N])->type;
            if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                matchedCoords.insert(ring[i]);
                matchedCoords.insert(ring[(i + 1) % N]);
                matchedCoords.insert(ring[(i + 2) % N]);
            }
        }
    };

    scanPerimeter(1, 6);
    scanPerimeter(0, 8);

    return !matchedCoords.empty();
}

void Board::applyGravityAndRefill() {
    // 1. Layer 2 Gravity Drop & Refill
    for (int c = 0; c < 4; ++c) {
        int writeRow = 3;
        for (int r = 3; r >= 0; --r) {
            ShapeTile* t = getTile({2, r, c});
            if (t->type != ShapeType::NONE) {
                if (writeRow != r) {
                    ShapeTile* targetTile = getTile({2, writeRow, c});
                    targetTile->type = t->type;
                    targetTile->scale = 1.0f;
                    targetTile->currentPos = t->currentPos;
                    targetTile->startMove(getSlotWorldPos(2, writeRow, c), 0.22f);
                    t->type = ShapeType::NONE;
                }
                writeRow--;
            }
        }
        for (int r = writeRow; r >= 0; --r) {
            ShapeTile* t = getTile({2, r, c});
            t->type = getRandomShapeType();
            t->scale = 1.0f;
            glm::vec3 spawnPos = getSlotWorldPos(2, r, c) + glm::vec3(0.0f, 1.8f, 0.0f);
            t->currentPos = spawnPos;
            t->startMove(getSlotWorldPos(2, r, c), 0.24f);
        }
    }

    // 2. Layer 1 & 0 Perimeter Refill
    auto refillPerimeter = [&](int layer, int size) {
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                BoardCoord coord{layer, r, c};
                if (!isValidCoord(coord)) continue;
                ShapeTile* t = getTile(coord);
                if (t->type == ShapeType::NONE) {
                    t->type = getRandomShapeType();
                    t->scale = 1.0f;
                    glm::vec3 spawnPos = getSlotWorldPos(layer, r, c) + glm::vec3(0.0f, 1.5f, 0.0f);
                    t->currentPos = spawnPos;
                    t->startMove(getSlotWorldPos(layer, r, c), 0.24f);
                }
            }
        }
    };
    refillPerimeter(1, 6);
    refillPerimeter(0, 8);
    std::cout << "  [Transform: Gravity Drop & Spawn] Shapes dropping along -Y into empty terrace slots" << std::endl;
}


void Board::update(float deltaTime) {
    // Smoothly animate Exploded View (E key)
    float targetExplode = isExploded ? 1.0f : 0.0f;
    if (std::abs(explodeProgress - targetExplode) > 0.0005f) {
        explodeProgress = glm::mix(explodeProgress, targetExplode, deltaTime * 5.0f);
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                ShapeTile* t = getTile({2, r, c});
                if (t && !t->isAnimating) {
                    t->currentPos = getSlotWorldPos(2, r, c);
                    t->targetPos  = t->currentPos;
                }
            }
        }
        for (int r = 0; r < 6; ++r) {
            for (int c = 0; c < 6; ++c) {
                if (isValidCoord({1, r, c})) {
                    ShapeTile* t = getTile({1, r, c});
                    if (t && !t->isAnimating) {
                        t->currentPos = getSlotWorldPos(1, r, c);
                        t->targetPos  = t->currentPos;
                    }
                }
            }
        }
    }

    // Update individual shape animations
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            getTile({2, r, c})->update(deltaTime);

    for (int r = 0; r < 6; ++r)
        for (int c = 0; c < 6; ++c)
            if (isValidCoord({1, r, c})) getTile({1, r, c})->update(deltaTime);

    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (isValidCoord({0, r, c})) getTile({0, r, c})->update(deltaTime);

    if (stateTimer > 0.0f) {
        stateTimer -= deltaTime;
    }

    switch (state) {
        case GameState::SWAPPING: {
            if (stateTimer <= 0.0f) {
                ShapeTile* tA = getTile(selectedCoord);
                ShapeTile* tB = getTile(swapTargetCoord);
                if (tA && tB) {
                    std::swap(tA->type, tB->type);
                    tA->currentPos = getSlotWorldPos(selectedCoord.layer, selectedCoord.row, selectedCoord.col);
                    tB->currentPos = getSlotWorldPos(swapTargetCoord.layer, swapTargetCoord.row, swapTargetCoord.col);
                }

                if (checkAndMarkMatches()) {
                    combo = 1;
                    int points = static_cast<int>(matchedCoords.size()) * 100 * combo;
                    score += points;
                    std::cout << "[MATCH] " << matchedCoords.size() << " shapes crushed! +" << points << " pts! (Combo " << combo << "x)" << std::endl;
                    std::cout << "  [Transform: Pop] Crushing matched shapes: Scale S( 1.000 -> 0.000 )" << std::endl;

                    for (const auto& c : matchedCoords) {
                        getTile(c)->startPop(0.22f);
                    }
                    state = GameState::POPPING;
                    stateTimer = 0.24f;
                } else {
                    std::cout << "[NO MATCH] Shapes did not line up. Reverting..." << std::endl;
                    executeSwap(selectedCoord, swapTargetCoord, true);
                }
            }
            break;
        }
        case GameState::REVERTING: {
            if (stateTimer <= 0.0f) {
                ShapeTile* tA = getTile(selectedCoord);
                ShapeTile* tB = getTile(swapTargetCoord);
                if (tA && tB) {
                    std::swap(tA->type, tB->type);
                    tA->currentPos = getSlotWorldPos(selectedCoord.layer, selectedCoord.row, selectedCoord.col);
                    tB->currentPos = getSlotWorldPos(swapTargetCoord.layer, swapTargetCoord.row, swapTargetCoord.col);
                }
                selectedCoord = BoardCoord();
                swapTargetCoord = BoardCoord();
                state = GameState::IDLE;
            }
            break;
        }
        case GameState::POPPING: {
            if (stateTimer <= 0.0f) {
                applyGravityAndRefill();
                state = GameState::FALLING;
                stateTimer = 0.26f;
            }
            break;
        }
        case GameState::FALLING: {
            if (stateTimer <= 0.0f) {
                if (checkAndMarkMatches()) {
                    combo++;
                    int points = static_cast<int>(matchedCoords.size()) * 150 * combo;
                    score += points;
                    std::cout << "[CASCADE COMBO " << combo << "x] +" << points << " pts!" << std::endl;
                    std::cout << "  [Transform: Pop] Crushing matched shapes: Scale S( 1.000 -> 0.000 )" << std::endl;

                    for (const auto& c : matchedCoords) {
                        getTile(c)->startPop(0.22f);
                    }
                    state = GameState::POPPING;
                    stateTimer = 0.24f;
                } else {
                    combo = 0;
                    selectedCoord = BoardCoord();
                    swapTargetCoord = BoardCoord();
                    state = GameState::IDLE;
                }
            }
            break;
        }
        default:
            break;
    }
}

void Board::draw(const Shader& shader, const Mesh& cubeMesh, const Mesh& sphereMesh,
                 const Mesh& cylinderMesh, const Mesh& diamondMesh, float time) const {
    // -------------------------------------------------------------
    // 1. DRAW STEPPED PYRAMID TRAYS (Blueprint Slate Wireframe)
    // -------------------------------------------------------------
    shader.setBool("isTray", true);
    shader.setVec4("objectColor", glm::vec4(0.38f, 0.42f, 0.52f, 1.0f));

    // Layer 0 Tray (Base: 8.5 x 0.25 x 8.5) at Y = 0.0
    glm::mat4 m0 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    m0 = glm::scale(m0, glm::vec3(8.5f, 0.25f, 8.5f));
    shader.setMat4("model", m0);
    cubeMesh.draw();

    // Layer 1 Tray (Middle: 6.5 x 0.25 x 6.5) at Y = 1.0 + explodeOffset
    float tray1Y = 1.0f + 1.25f * explodeProgress;
    glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, tray1Y, 0.0f));
    m1 = glm::scale(m1, glm::vec3(6.5f, 0.25f, 6.5f));
    shader.setMat4("model", m1);
    cubeMesh.draw();

    // Layer 2 Tray (Top: 4.5 x 0.25 x 4.5) at Y = 2.0 + explodeOffset
    float tray2Y = 2.0f + 2.50f * explodeProgress;
    glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, tray2Y, 0.0f));
    m2 = glm::scale(m2, glm::vec3(4.5f, 0.25f, 4.5f));
    shader.setMat4("model", m2);
    cubeMesh.draw();

    // -------------------------------------------------------------
    // 2. DRAW SHAPES ACROSS ALL TIERS (Pure White Crisp Wireframe)
    // -------------------------------------------------------------
    shader.setBool("isTray", false);

    // Draw Top Layer (4x4)
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            BoardCoord coord{2, r, c};
            bool isSel = (coord == selectedCoord);
            getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
        }
    }

    // Draw Middle Layer (6x6 perimeter)
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 6; ++c) {
            BoardCoord coord{1, r, c};
            if (!isValidCoord(coord)) continue;
            bool isSel = (coord == selectedCoord);
            getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
        }
    }

    // Draw Base Layer (8x8 perimeter)
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            BoardCoord coord{0, r, c};
            if (!isValidCoord(coord)) continue;
            bool isSel = (coord == selectedCoord);
            getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
        }
    }
}

```

---

<a id="srcmaincpp"></a>
### `src/main.cpp`
**Description:** Application entry, GLFW window setup, picking event loop, and render cycle

```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <cstdio>

#include "Mesh.h"
#include "Primitives.h"
#include "Shader.h"
#include "Camera.h"
#include "Board.h"

// -----------------------------------------------------------------------------
// Global Application State
// -----------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

// Wireframe state: ON by default for pure geometric inspection
bool g_wireframe = true;
bool g_orthographic = false;

Camera g_camera;
Board  g_board;

// Mouse navigation
bool g_isRightDragging = false;
double g_lastMouseX = 0.0;
double g_lastMouseY = 0.0;

int g_lastScore = -1;
int g_lastCombo = -1;

// -----------------------------------------------------------------------------
// GLFW Event Callbacks
// -----------------------------------------------------------------------------
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        } else if (key == GLFW_KEY_W) {
            g_wireframe = !g_wireframe;
            std::cout << "[Mode] " << (g_wireframe ? "PURE WIREFRAME (GL_LINE)" : "MINIMAL CLAY (GL_FILL)") << std::endl;
        } else if (key == GLFW_KEY_E) {
            g_board.toggleExplode();
        } else if (key == GLFW_KEY_SPACE) {
            g_camera.autoOrbit = !g_camera.autoOrbit;
            std::cout << "[Camera] Auto-Orbit " << (g_camera.autoOrbit ? "RESUMED" : "PAUSED") << std::endl;
        } else if (key == GLFW_KEY_P) {
            g_orthographic = !g_orthographic;
            std::cout << "[Camera] Projection: " << (g_orthographic ? "ORTHOGRAPHIC" : "PERSPECTIVE") << std::endl;
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            float aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : 1.0f;
            if (g_orthographic) {
                float orthoSize = g_camera.distance * 0.45f;
                std::cout << "  [Transform: Projection - Orthographic]\n"
                          << "    Bounds: Left: " << -orthoSize * aspect << ", Right: " << orthoSize * aspect
                          << ", Bottom: " << -orthoSize << ", Top: " << orthoSize << ", Near: 0.1, Far: 100.0" << std::endl;
            } else {
                std::cout << "  [Transform: Projection - Perspective]\n"
                          << "    FOV: 45.0 deg, Aspect: " << aspect << ", Near: 0.1, Far: 100.0" << std::endl;
            }
        } else if (key == GLFW_KEY_M) {
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            float aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : 1.0f;

            glm::mat4 view = g_camera.getViewMatrix();
            glm::mat4 proj;
            if (g_orthographic) {
                float orthoSize = g_camera.distance * 0.45f;
                proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
            } else {
                proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            }

            glm::mat4 model(1.0f);
            std::string modelDesc = "Pyramid Base (Identity Model Matrix)";
            if (g_board.selectedCoord.isValid()) {
                const ShapeTile* t = g_board.getTile(g_board.selectedCoord);
                if (t) {
                    model = t->getModelMatrix(static_cast<float>(glfwGetTime()), true);
                    modelDesc = "Selected Tile: " + getShapeName(t->type) + " at Layer " +
                                std::to_string(g_board.selectedCoord.layer) + " (" +
                                std::to_string(g_board.selectedCoord.row) + ", " +
                                std::to_string(g_board.selectedCoord.col) + ")";
                }
            }

            glm::mat4 mvp = proj * view * model;

            std::cout << "\n==================================================================" << std::endl;
            std::cout << "       4x4 MODEL-VIEW-PROJECTION (MVP) MATRIX INSPECTION" << std::endl;
            std::cout << "==================================================================" << std::endl;

            glm::vec3 eye = g_camera.getPosition();
            std::cout << "CAMERA EYE POS : [ " << eye.x << ", " << eye.y << ", " << eye.z << " ]\n"
                      << "LOOK-AT TARGET : [ " << g_camera.target.x << ", " << g_camera.target.y << ", " << g_camera.target.z << " ]\n"
                      << "DISTANCE       : " << g_camera.distance << " | YAW: " << g_camera.yaw << " deg | PITCH: " << g_camera.pitch << " deg\n"
                      << "PROJECTION     : " << (g_orthographic ? "Orthographic" : "Perspective (FOV: 45.0 deg)") << "\n"
                      << "TARGET OBJECT  : " << modelDesc << "\n" << std::endl;

            auto printMat = [](const char* name, const glm::mat4& m) {
                std::cout << name << " (4x4):\n";
                for (int r = 0; r < 4; ++r) {
                    std::cout << "  [ ";
                    for (int c = 0; c < 4; ++c) {
                        std::printf("%8.3f ", m[c][r]);
                    }
                    std::cout << "]\n";
                }
                std::cout << std::endl;
            };

            printMat("1. MODEL MATRIX [M]", model);
            printMat("2. VIEW MATRIX [V]", view);
            printMat("3. PROJECTION MATRIX [P]", proj);
            printMat("4. COMBINED MVP MATRIX [P * V * M]", mvp);
            std::cout << "==================================================================\n" << std::endl;
        } else if (key == GLFW_KEY_R) {
            g_board.reset();
            std::cout << "[Game] Board reshuffled and score reset." << std::endl;
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : 1.0f;

        glm::mat4 proj;
        if (g_orthographic) {
            float orthoSize = g_camera.distance * 0.45f;
            proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
        } else {
            proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        }

        // Raycast from 2D screen into 3D world space
        Ray ray = g_camera.getRayFromScreen(mx, my, w, h, proj);
        BoardCoord hit = g_board.pickTile(ray);
        g_board.onTileClicked(hit);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            g_isRightDragging = true;
            glfwGetCursorPos(window, &g_lastMouseX, &g_lastMouseY);
        } else if (action == GLFW_RELEASE) {
            g_isRightDragging = false;
            glm::vec3 eye = g_camera.getPosition();
            std::cout << "[Transform: Camera View] Eye: [ " << eye.x << ", " << eye.y << ", " << eye.z
                      << " ] | Yaw: " << g_camera.yaw << " deg | Pitch: " << g_camera.pitch << " deg" << std::endl;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (g_isRightDragging) {
        float xoffset = static_cast<float>(xpos - g_lastMouseX);
        float yoffset = static_cast<float>(g_lastMouseY - ypos);
        g_lastMouseX = xpos;
        g_lastMouseY = ypos;

        g_camera.processMouseDrag(xoffset, yoffset);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window; (void)xoffset;
    g_camera.processScroll(static_cast<float>(yoffset));
    std::cout << "[Transform: Camera Zoom] Distance: " << g_camera.distance << " units" << std::endl;
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

// -----------------------------------------------------------------------------
// Main Application Entry
// -----------------------------------------------------------------------------
int main() {
    // 1. Initialize GLFW (prefer X11 backend on Fedora/Wayland for GLEW compatibility)
#if defined(GLFW_PLATFORM) && defined(GLFW_PLATFORM_X11)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // Anti-Aliasing

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Geometric Shape-Match (Pure Wireframe)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 60 FPS V-Sync

    // Callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // 2. Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // 3. Configure OpenGL Pipeline
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    // 4. Compile & Link Shaders
    Shader shader;
    if (!shader.init("shaders/wireframe.vert", "shaders/wireframe.frag")) {
        std::cerr << "Failed to build shader program!" << std::endl;
        return -1;
    }

    // 5. Generate Procedural Geometric Meshes (All pure GL_TRIANGLES)
    Mesh cubeMesh     = Primitives::createCube();                    // Unit Cube (12 tris)
    Mesh cylinderMesh = Primitives::createCylinder(8, 0.4f, 0.35f);  // Octagonal Prism (32 tris)
    Mesh sphereMesh   = Primitives::createSphere(16, 20, 0.42f);     // UV Sphere (640 tris)
    Mesh diamondMesh  = Primitives::createDiamond(8, 0.42f, 0.65f);  // Bipyramid (16 tris)

    // Print Console Instructions
    std::cout << "\n==================================================================\n"
              << "       3D GEOMETRIC SHAPE-MATCH: PURE WIREFRAME EDITION\n"
              << "==================================================================\n"
              << " OBJECTIVE:\n"
              << "   Match 3 of the same 3D geometric shape in a row:\n"
              << "     [1] UV Sphere     : 16 rings x 20 sectors = 640 Triangles\n"
              << "     [2] Oct. Cylinder : 8 sectors + 2 fan caps = 32 Triangles\n"
              << "     [3] Unit Cube     : 6 faces x 2 triangles = 12 Triangles\n"
              << "     [4] Bipyramid     : 8 equatorial facets    = 16 Triangles\n"
              << " CONTROLS:\n"
              << "   - Left Mouse Click   : Select Shape & Click adjacent neighbor to SWAP\n"
              << "   - Match-3 Rule       : Align 3 identical 3D shapes to crush & drop\n"
              << "   - [E] Key            : Exploded Layer View (Separates Tiers along Y)\n"
              << "   - [W] Key            : Toggle Wireframe (Default: ON) / Flat Solid\n"
              << "   - [P] Key            : Toggle Perspective / Orthographic View\n"
              << "   - [M] Key            : Dump 4x4 Model-View-Projection (MVP) Matrices\n"
              << "   - [Spacebar]         : Pause / Resume 360 Camera Auto-Orbit\n"
              << "   - [R] Key            : Reshuffle & Reset Board\n"
              << "   - Right Mouse Drag   : 360 Manual Orbit\n"
              << "   - Mouse Scroll Wheel : Zoom In / Out\n"
              << "   - [Esc] Key          : Exit Game\n"
              << "==================================================================\n" << std::endl;

    double lastTime = glfwGetTime();

    // -------------------------------------------------------------------------
    // Main Render Loop
    // -------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Update Subsystems
        g_camera.update(deltaTime);
        g_board.update(deltaTime);

        // Update Dynamic Window Title
        if (g_board.score != g_lastScore || g_board.combo != g_lastCombo) {
            char title[256];
            std::snprintf(title, sizeof(title),
                          "3D SHAPE-MATCH (WIREFRAME) | Score: %d | Combo: %dx | [E] Explode | [W] Mode",
                          g_board.score, g_board.combo);
            glfwSetWindowTitle(window, title);
            g_lastScore = g_board.score;
            g_lastCombo = g_board.combo;
        }

        // Toggle Polygon Mode
        if (g_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE); // See through all triangles in wireframe mode
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }

        // Clear screen with deep dark slate background
        glClearColor(0.06f, 0.065f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix
        glm::mat4 view = g_camera.getViewMatrix();

        // Projection Matrix
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (height > 0) ? (static_cast<float>(width) / static_cast<float>(height)) : 1.0f;

        glm::mat4 projection;
        if (g_orthographic) {
            float orthoSize = g_camera.distance * 0.45f;
            projection = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
        } else {
            projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        }

        shader.use();
        shader.setBool("isWireframe", g_wireframe);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // Draw Stepped Pyramid Trays and Geometric Shapes
        g_board.draw(shader, cubeMesh, sphereMesh, cylinderMesh, diamondMesh, static_cast<float>(currentTime));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup Resources
    cubeMesh.destroy();
    cylinderMesh.destroy();
    sphereMesh.destroy();
    diamondMesh.destroy();
    shader.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

```

---

## 🛠️ Fedora Compilation & Execution Instructions

### Option 1: Fast Build with `make` (Recommended)
```bash
cd candy_pyramid
make clean && make run
```

### Option 2: Using the Shell Script
```bash
cd candy_pyramid
chmod +x build.sh
./build.sh
```

### Option 3: Modern `CMake` Build
```bash
cd candy_pyramid
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./candy_pyramid
```
