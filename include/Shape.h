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
glm::vec4 getShapeColor(ShapeType type);

struct ShapeTile {
    ShapeType type = ShapeType::NONE;
    int layer = 0;
    int row = 0;
    int col = 0;
    int level = 0;
    bool isGolden = false; // Rare high-value bonus shape (500 pts)

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

    void draw(const Shader& shader, const Mesh& bentCylMesh, const Mesh& saucerMesh,
              const Mesh& pillowMesh, const Mesh& teardropMesh, bool isSelected, float time) const;
};

