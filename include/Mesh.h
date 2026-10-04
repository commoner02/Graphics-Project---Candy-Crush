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
