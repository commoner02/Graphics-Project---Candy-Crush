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
