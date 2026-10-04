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

glm::vec4 getShapeColor(ShapeType type) {
    switch (type) {
        case ShapeType::SPHERE:   return glm::vec4(1.00f, 0.28f, 0.40f, 1.0f); // Ruby Red / Crimson
        case ShapeType::CYLINDER: return glm::vec4(0.18f, 0.90f, 0.52f, 1.0f); // Emerald Mint Green
        case ShapeType::CUBE:     return glm::vec4(1.00f, 0.76f, 0.16f, 1.0f); // Topaz Amber Gold
        case ShapeType::DIAMOND:  return glm::vec4(0.22f, 0.80f, 1.00f, 1.0f); // Electric Cyan / Sapphire
        default:                  return glm::vec4(0.85f, 0.85f, 0.85f, 1.0f);
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

    glm::vec4 color = getShapeColor(type);
    if (isSelected) {
        // Dynamic pulsating golden-white selection highlight
        float pulse = 0.5f + 0.5f * std::sin(time * 8.0f);
        color = glm::mix(color, glm::vec4(1.0f, 1.0f, 0.80f, 1.0f), 0.65f + 0.35f * pulse);
    }
    shader.setVec4("objectColor", color);

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
