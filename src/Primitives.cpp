#include "Primitives.h"
#include <cmath>
#include <vector>
#include <glm/glm.hpp>

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

        // Two non-overlapping triangles per quad sector (CCW outward)
        indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
        indices.push_back(i1); indices.push_back(i2); indices.push_back(i3);
    }

    // --- Top Cap (Triangulated from central apex, CCW outward) ---
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
        indices.push_back(topRimStart + i + 1);
        indices.push_back(topRimStart + i);
    }

    // --- Bottom Cap (Triangulated from central apex, CCW outward) ---
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
        indices.push_back(botRimStart + i);
        indices.push_back(botRimStart + i + 1);
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

    // Generate non-overlapping triangles with CCW outward winding (no pole degenerates)
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first = static_cast<unsigned int>((r * (sectors + 1)) + s);
            unsigned int second = first + static_cast<unsigned int>(sectors + 1);

            if (r != rings - 1) {
                // Triangle 1 (CCW outward facing): first -> second+1 -> second
                indices.push_back(first);
                indices.push_back(second + 1);
                indices.push_back(second);
            }

            if (r != 0) {
                // Triangle 2 (CCW outward facing): first -> first+1 -> second+1
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second + 1);
            }
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

        // Top triangle fan (CCW outward)
        indices.push_back(topApexIdx);
        indices.push_back(next);
        indices.push_back(curr);

        // Bottom triangle fan (CCW outward)
        indices.push_back(botApexIdx);
        indices.push_back(curr);
        indices.push_back(next);
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 5. RED BENT CYLINDER (Thin Cylinder Bent to Look Like a Jelly Bean with Rounded Half-Circle Caps)
// -----------------------------------------------------------------------------
Mesh createBentCylinder(int spineSegments, int tubeSegments, int capRings, float halfLength, float radius, float bendDepth) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float pi2 = 2.0f * static_cast<float>(M_PI);
    int stride = tubeSegments + 1;

    // 1. Tube body rings along quadratic spine curve
    // The spine curves upward: concave inner curve faces UPWARD (+Y)
    for (int i = 0; i <= spineSegments; ++i) {
        float t = -1.0f + 2.0f * (static_cast<float>(i) / static_cast<float>(spineSegments)); // -1 to +1

        // Spine position: dips at t=0, curls up at t=+-1
        float cx = halfLength * t;
        float cy = bendDepth * (t * t - 0.5f);
        float cz = 0.0f;

        // Tangent vector
        float tx = halfLength;
        float ty = 2.0f * bendDepth * t;
        float tLen = std::sqrt(tx * tx + ty * ty);
        tx /= tLen;
        ty /= tLen;

        // Inward/upward normal in XY plane
        float nx = -ty;
        float ny = tx;

        // Binormal along Z
        float bx = 0.0f;
        float by = 0.0f;
        float bz = 1.0f;

        for (int j = 0; j <= tubeSegments; ++j) {
            float theta = pi2 * static_cast<float>(j) / static_cast<float>(tubeSegments);
            float cost = std::cos(theta);
            float sint = std::sin(theta);

            // Tube cross-section surface normal
            float snx = cost * nx + sint * bx;
            float sny = cost * ny + sint * by;
            float snz = cost * 0.0f + sint * bz;

            // Surface vertex position
            float px = cx + radius * snx;
            float py = cy + radius * sny;
            float pz = cz + radius * snz;

            vertices.push_back(px);
            vertices.push_back(py);
            vertices.push_back(pz);
            vertices.push_back(snx);
            vertices.push_back(sny);
            vertices.push_back(snz);
        }
    }

    // Tube side quads (CCW outward)
    for (int i = 0; i < spineSegments; ++i) {
        for (int j = 0; j < tubeSegments; ++j) {
            unsigned int first = static_cast<unsigned int>(i * stride + j);
            unsigned int second = static_cast<unsigned int>((i + 1) * stride + j);

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);

            indices.push_back(first + 1);
            indices.push_back(second + 1);
            indices.push_back(second);
        }
    }

    float tEndLen = std::sqrt(halfLength * halfLength + 4.0f * bendDepth * bendDepth);

    // 2. Start Hemisphere Cap (t = -1, rounded half-circle dome facing -Tangent)
    glm::vec3 capOutStart(-halfLength / tEndLen, 2.0f * bendDepth / tEndLen, 0.0f);
    glm::vec3 cStart(-halfLength, bendDepth * 0.5f, 0.0f);
    glm::vec3 nStart(2.0f * bendDepth / tEndLen, halfLength / tEndLen, 0.0f);
    glm::vec3 bStart(0.0f, 0.0f, 1.0f);

    unsigned int startBase = static_cast<unsigned int>(vertices.size() / 6);
    for (int k = 0; k < capRings; ++k) {
        float phi = (static_cast<float>(M_PI) * 0.5f) * (static_cast<float>(k) / static_cast<float>(capRings));
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (int j = 0; j <= tubeSegments; ++j) {
            float theta = pi2 * static_cast<float>(j) / static_cast<float>(tubeSegments);
            glm::vec3 rad = std::cos(theta) * nStart + std::sin(theta) * bStart;
            glm::vec3 norm = glm::normalize(cosPhi * rad + sinPhi * capOutStart);
            glm::vec3 p = cStart + radius * norm;

            vertices.push_back(p.x);
            vertices.push_back(p.y);
            vertices.push_back(p.z);
            vertices.push_back(norm.x);
            vertices.push_back(norm.y);
            vertices.push_back(norm.z);
        }
    }

    // Start apex pole vertex
    unsigned int startApexIdx = static_cast<unsigned int>(vertices.size() / 6);
    glm::vec3 pApexStart = cStart + radius * capOutStart;
    vertices.push_back(pApexStart.x);
    vertices.push_back(pApexStart.y);
    vertices.push_back(pApexStart.z);
    vertices.push_back(capOutStart.x);
    vertices.push_back(capOutStart.y);
    vertices.push_back(capOutStart.z);

    // Quads between start hemisphere rings
    for (int k = 0; k < capRings - 1; ++k) {
        for (int j = 0; j < tubeSegments; ++j) {
            unsigned int r0 = startBase + static_cast<unsigned int>(k * stride + j);
            unsigned int r1 = startBase + static_cast<unsigned int>((k + 1) * stride + j);
            indices.push_back(r0);
            indices.push_back(r1);
            indices.push_back(r0 + 1);

            indices.push_back(r0 + 1);
            indices.push_back(r1);
            indices.push_back(r1 + 1);
        }
    }

    // Triangle fan to start apex pole
    unsigned int lastStartRing = startBase + static_cast<unsigned int>((capRings - 1) * stride);
    for (int j = 0; j < tubeSegments; ++j) {
        indices.push_back(lastStartRing + j);
        indices.push_back(startApexIdx);
        indices.push_back(lastStartRing + j + 1);
    }

    // 3. End Hemisphere Cap (t = +1, rounded half-circle dome facing +Tangent)
    glm::vec3 capOutEnd(halfLength / tEndLen, 2.0f * bendDepth / tEndLen, 0.0f);
    glm::vec3 cEnd(halfLength, bendDepth * 0.5f, 0.0f);
    glm::vec3 nEnd(-2.0f * bendDepth / tEndLen, halfLength / tEndLen, 0.0f);
    glm::vec3 bEnd(0.0f, 0.0f, 1.0f);

    unsigned int endBase = static_cast<unsigned int>(vertices.size() / 6);
    for (int k = 0; k < capRings; ++k) {
        float phi = (static_cast<float>(M_PI) * 0.5f) * (static_cast<float>(k) / static_cast<float>(capRings));
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (int j = 0; j <= tubeSegments; ++j) {
            float theta = pi2 * static_cast<float>(j) / static_cast<float>(tubeSegments);
            glm::vec3 rad = std::cos(theta) * nEnd + std::sin(theta) * bEnd;
            glm::vec3 norm = glm::normalize(cosPhi * rad + sinPhi * capOutEnd);
            glm::vec3 p = cEnd + radius * norm;

            vertices.push_back(p.x);
            vertices.push_back(p.y);
            vertices.push_back(p.z);
            vertices.push_back(norm.x);
            vertices.push_back(norm.y);
            vertices.push_back(norm.z);
        }
    }

    // End apex pole vertex
    unsigned int endApexIdx = static_cast<unsigned int>(vertices.size() / 6);
    glm::vec3 pApexEnd = cEnd + radius * capOutEnd;
    vertices.push_back(pApexEnd.x);
    vertices.push_back(pApexEnd.y);
    vertices.push_back(pApexEnd.z);
    vertices.push_back(capOutEnd.x);
    vertices.push_back(capOutEnd.y);
    vertices.push_back(capOutEnd.z);

    // Quads between end hemisphere rings
    for (int k = 0; k < capRings - 1; ++k) {
        for (int j = 0; j < tubeSegments; ++j) {
            unsigned int r0 = endBase + static_cast<unsigned int>(k * stride + j);
            unsigned int r1 = endBase + static_cast<unsigned int>((k + 1) * stride + j);
            indices.push_back(r0);
            indices.push_back(r0 + 1);
            indices.push_back(r1);

            indices.push_back(r0 + 1);
            indices.push_back(r1 + 1);
            indices.push_back(r1);
        }
    }

    // Triangle fan to end apex pole
    unsigned int lastEndRing = endBase + static_cast<unsigned int>((capRings - 1) * stride);
    for (int j = 0; j < tubeSegments; ++j) {
        indices.push_back(lastEndRing + j);
        indices.push_back(lastEndRing + j + 1);
        indices.push_back(endApexIdx);
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 9. ORANGE WATER DROP (Bulbous Plump Base Tapering to Smooth Rounded Apex Tip)
// -----------------------------------------------------------------------------
Mesh createTeardrop(int rings, int sectors, float radius, float height) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float pi = static_cast<float>(M_PI);
    float pi2 = 2.0f * pi;
    float halfH = height * 0.5f;

    // Natural graceful water drop profile:
    // phi goes from 0 (top rounded apex dome) to PI (bottom bulbous base pole).
    // Y(phi) = halfH * cos(phi)
    // Profile R(phi) = sin(phi) * g(phi), where g(phi) transitions smoothly
    // from a gentle rounded tip dome (radius ~ 0.15) to a plump bulbous base (radius ~ 0.55).
    // The top apex is a smooth, continuous rounded dome with zero sharp spike!
    float a = 0.15f * (radius / 0.35f);  // Smooth rounded tip dome scale
    float b = 0.40f * (radius / 0.35f);  // Plump bulbous base expansion scale
    float p1 = 1.35f;

    for (int r = 0; r <= rings; ++r) {
        float phi = pi * static_cast<float>(r) / static_cast<float>(rings);
        float sinP = std::sin(phi);
        float cosP = std::cos(phi);

        float t = 0.5f * (1.0f - cosP); // 0 at top apex, 1 at bottom base
        float dt_dphi = 0.5f * sinP;

        float t_p1 = std::pow(std::max(0.0f, t), p1);

        float g = a + b * t_p1;

        float dg_dt = 0.0f;
        if (t > 1e-5f) {
            dg_dt = b * p1 * std::pow(t, p1 - 1.0f);
        }
        float dg_dphi = dg_dt * dt_dphi;

        float profileR = sinP * g;
        float y = halfH * cosP;

        // Exact analytical derivatives for smooth, continuous lighting normals
        float dY_dPhi = -halfH * sinP;
        float dR_dPhi = cosP * g + sinP * dg_dphi;

        // Outward 2D normal: (-dY_dPhi, dR_dPhi)
        glm::vec2 n2D(-dY_dPhi, dR_dPhi);
        float nLen = glm::length(n2D);
        if (nLen > 1e-5f) {
            n2D /= nLen;
        } else {
            n2D = glm::vec2(0.0f, 1.0f);
        }

        for (int sIdx = 0; sIdx <= sectors; ++sIdx) {
            float theta = pi2 * static_cast<float>(sIdx) / static_cast<float>(sectors);
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            float x = profileR * cosT;
            float z = profileR * sinT;

            float nx = n2D.x * cosT;
            float ny = n2D.y;
            float nz = n2D.x * sinT;

            if (r == 0)     { nx = 0.0f; ny = 1.0f; nz = 0.0f; }  // Smooth top dome pole
            if (r == rings) { nx = 0.0f; ny = -1.0f; nz = 0.0f; } // Smooth bottom base pole

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int sIdx = 0; sIdx < sectors; ++sIdx) {
            unsigned int first = static_cast<unsigned int>(r * (sectors + 1) + sIdx);
            unsigned int second = static_cast<unsigned int>((r + 1) * (sectors + 1) + sIdx);

            if (r != 0) {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second + 1);
            }

            if (r != rings - 1) {
                indices.push_back(first);
                indices.push_back(second + 1);
                indices.push_back(second);
            }
        }
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 10. BLUE FLYING SAUCER / SATURN DROP (Sphere with Equatorial Ridge Collar)
// -----------------------------------------------------------------------------
Mesh createFlyingSaucer(int rings, int sectors, float radius, float flange) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float pi = static_cast<float>(M_PI);
    float pi2 = 2.0f * pi;
    float sigma = 0.22f;
    float twoSigmaSq = 2.0f * sigma * sigma;

    for (int r = 0; r <= rings; ++r) {
        float phi = pi * static_cast<float>(r) / static_cast<float>(rings); // 0 (North) to PI (South)
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        float dPhi = phi - 0.5f * pi;
        float gauss = std::exp(-(dPhi * dPhi) / twoSigmaSq);
        float profileR = radius * sinPhi + flange * gauss;
        float y = 0.82f * radius * cosPhi;

        // Outward normal from curve slope
        float dR_dPhi = radius * cosPhi - flange * (dPhi / (sigma * sigma)) * gauss;
        float dY_dPhi = -0.82f * radius * sinPhi;
        glm::vec2 n2D = glm::normalize(glm::vec2(-dY_dPhi, dR_dPhi));

        for (int s = 0; s <= sectors; ++s) {
            float theta = pi2 * static_cast<float>(s) / static_cast<float>(sectors);
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            float x = profileR * cosT;
            float z = profileR * sinT;

            float nx = n2D.x * cosT;
            float ny = n2D.y;
            float nz = n2D.x * sinT;

            if (r == 0) { nx = 0.0f; ny = 1.0f; nz = 0.0f; }
            if (r == rings) { nx = 0.0f; ny = -1.0f; nz = 0.0f; }

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first = static_cast<unsigned int>(r * (sectors + 1) + s);
            unsigned int second = first + static_cast<unsigned int>(sectors + 1);

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second + 1);

            indices.push_back(first);
            indices.push_back(second + 1);
            indices.push_back(second);
        }
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

// -----------------------------------------------------------------------------
// 11. GREEN PUFFED SQUARE PILLOW (Superquadric Cushion Confection)
// -----------------------------------------------------------------------------
Mesh createPillow(int rings, int sectors, float halfWidth, float halfHeight) {
    Mesh mesh;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float pi = static_cast<float>(M_PI);
    float pi2 = 2.0f * pi;
    float s1 = 0.45f; // Vertical squish / rounding
    float s2 = 0.35f; // Horizontal squareness

    auto sgnPow = [](float val, float p) -> float {
        float s = (val >= 0.0f) ? 1.0f : -1.0f;
        return s * std::pow(std::abs(val), p);
    };

    for (int r = 0; r <= rings; ++r) {
        float eta = -0.5f * pi + pi * static_cast<float>(r) / static_cast<float>(rings); // -PI/2 to +PI/2
        float cosEta = std::cos(eta);
        float sinEta = std::sin(eta);

        float sgnCosEta = sgnPow(cosEta, s1);
        float sgnSinEta = sgnPow(sinEta, s1);

        for (int s = 0; s <= sectors; ++s) {
            float omega = -pi + pi2 * static_cast<float>(s) / static_cast<float>(sectors); // -PI to +PI
            float cosOm = std::cos(omega);
            float sinOm = std::sin(omega);

            float sgnCosOm = sgnPow(cosOm, s2);
            float sgnSinOm = sgnPow(sinOm, s2);

            float x = halfWidth * sgnCosEta * sgnCosOm;
            float y = halfHeight * sgnSinEta;
            float z = halfWidth * sgnCosEta * sgnSinOm;

            // Analytical Superquadric outward normal
            float nx = (1.0f / halfWidth) * sgnPow(cosEta, 2.0f - s1) * sgnPow(cosOm, 2.0f - s2);
            float ny = (1.0f / halfHeight) * sgnPow(sinEta, 2.0f - s1);
            float nz = (1.0f / halfWidth) * sgnPow(cosEta, 2.0f - s1) * sgnPow(sinOm, 2.0f - s2);
            glm::vec3 n = glm::normalize(glm::vec3(nx, ny, nz));

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            unsigned int first = static_cast<unsigned int>(r * (sectors + 1) + s);
            unsigned int second = first + static_cast<unsigned int>(sectors + 1);

            indices.push_back(first);
            indices.push_back(second + 1);
            indices.push_back(first + 1);

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(second + 1);
        }
    }

    mesh.setupBuffers(vertices, indices);
    return mesh;
}

} // namespace Primitives
