#include "Shape.h"
#include <string>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::string getShapeName(ShapeType type) {
    switch (type) {
        case ShapeType::SPHERE:   return "CHERRY BENT CYLINDER";
        case ShapeType::CYLINDER: return "BLUE FLYING SAUCER";
        case ShapeType::CUBE:     return "LIME SQUARE PILLOW";
        case ShapeType::DIAMOND:  return "YELLOW WATER DROP";
        default:                  return "EMPTY";
    }
}

glm::vec4 getShapeColor(ShapeType type) {
    switch (type) {
        case ShapeType::SPHERE:   return glm::vec4(1.00f, 0.16f, 0.28f, 1.0f); // Juicy Candied Cherry Red Bent Cylinder
        case ShapeType::CYLINDER: return glm::vec4(0.08f, 0.40f, 0.88f, 1.0f); // Deep Sapphire Blue Flying Saucer
        case ShapeType::CUBE:     return glm::vec4(0.06f, 0.65f, 0.20f, 1.0f); // Rich Candied Emerald Green Pillow
        case ShapeType::DIAMOND:  return glm::vec4(1.00f, 0.88f, 0.12f, 1.0f); // Bright Candied Lemon Yellow Drop
        default:                  return glm::vec4(0.95f, 0.95f, 0.95f, 1.0f);
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
    scale = 1.0f;
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
        if (!isPopping) {
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

    // - Red Bent Cylinder: Aligned to cube diagonal (45 deg around Y), concave inner curve facing upward (+Y)
    // - Orange Water Drop: Stands upright facing upward (+Y), sleek tip pointing to the sky
    if (type == ShapeType::SPHERE) {
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (isPopping) {
        float t = animProgress;

        if (isGolden) {
            // 24K Gold Supernova Pop: Radiant 1.6x swell, double-axis ascending spiral
            float swell = (t < 0.35f) ? (1.0f + 0.60f * std::sin((t / 0.35f) * 1.5707963f))
                                      : (1.60f * (1.0f - std::pow((t - 0.35f) / 0.65f, 2.0f)));
            float liftY = 0.35f * std::sin(static_cast<float>(M_PI) * t);
            model = glm::translate(model, glm::vec3(0.0f, liftY, 0.0f));
            model = glm::rotate(model, t * 24.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, t * 8.0f, glm::vec3(1.0f, 0.0f, 1.0f));
            return glm::scale(model, glm::vec3(swell * 0.95f));
        }

        switch (type) {
            case ShapeType::SPHERE: {
                // 1. Red Bent Cylinder: Dynamic spring bounce flex pop with upward hop
                float swell;
                if (t < 0.35f) {
                    float p = t / 0.35f;
                    swell = 1.0f + 0.40f * std::sin(p * 1.5707963f);
                } else {
                    float p = (t - 0.35f) / 0.65f;
                    swell = 1.40f * (1.0f - p * p);
                }
                float liftY = 0.25f * std::sin(static_cast<float>(M_PI) * t);
                model = glm::translate(model, glm::vec3(0.0f, liftY, 0.0f));
                model = glm::rotate(model, t * 16.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                float flex = 1.0f + 0.20f * std::sin(t * 16.0f);
                return glm::scale(model, glm::vec3(swell * flex, swell * (2.0f - flex), swell));
            }

            case ShapeType::CYLINDER: {
                // 2. Blue Flying Saucer: High-speed 28 rad/s turntable spin + pancake collar compression
                float radialSwell = (t < 0.25f) ? (1.0f + 0.40f * (t / 0.25f)) : (1.40f * (1.0f - std::pow((t - 0.25f) / 0.75f, 2.0f)));
                float verticalSquash = std::max(0.05f, 1.0f - t * 0.85f);
                model = glm::rotate(model, t * 28.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                return glm::scale(model, glm::vec3(radialSwell, radialSwell * verticalSquash, radialSwell));
            }

            case ShapeType::CUBE: {
                // 3. Green Square Pillow: Puffed pillow 1.3x swell + 3D chaotic tumbling roll
                float swell = (t < 0.25f) ? (1.0f + 0.30f * (t / 0.25f)) : (1.30f * (1.0f - std::pow((t - 0.25f) / 0.75f, 2.0f)));
                float hopY = 0.25f * std::sin(static_cast<float>(M_PI) * t);
                model = glm::translate(model, glm::vec3(0.0f, hopY, 0.0f));
                model = glm::rotate(model, t * 14.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, t * 18.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                return glm::scale(model, glm::vec3(swell));
            }

            case ShapeType::DIAMOND: {
                // 4. Orange Water Drop: Supersonic droplet rocket launch with vertical stretch
                float swell = (t < 0.25f) ? (1.0f + 0.30f * (t / 0.25f)) : (1.30f * (1.0f - std::pow((t - 0.25f) / 0.75f, 2.0f)));
                float rocketLaunchY = 0.45f * t;
                model = glm::translate(model, glm::vec3(0.0f, rocketLaunchY, 0.0f));
                model = glm::rotate(model, t * 36.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                return glm::scale(model, glm::vec3(swell * 0.85f, swell * 1.45f, swell * 0.85f));
            }

            default:
                return glm::scale(model, glm::vec3(std::max(0.0f, 1.0f - t)));
        }
    }

    if (isSelected) {
        // Selected tile bounces up and rotates smoothly
        float hoverY = 0.15f + 0.05f * std::sin(time * 8.0f);
        model = glm::translate(model, glm::vec3(0.0f, hoverY, 0.0f));
        model = glm::rotate(model, time * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    } else if (!isAnimating) {
        // ---------------------------------------------------------------
        // PER-CANDY IDLE MOTIONS (Unique transform per Candy Crush sweet)
        //   Red Cylinder → gentle hover drift + smooth turntable Y-spin (+ breathing pulse)
        //   Blue Saucer  → continuous turntable spin + 18deg collar tilt
        //   Green Pillow → rocking cushion wobble tilt
        //   Orange Drop  → floating bob + swaying pendulum tilt
        // ---------------------------------------------------------------
        switch (type) {
            case ShapeType::SPHERE: {
                // Red Bent Cylinder: gentle hover drift + smooth turntable Y-spin
                float driftY = 0.04f * std::sin(time * 2.5f + currentPos.x * 1.1f + currentPos.z * 1.4f);
                model = glm::translate(model, glm::vec3(0.0f, driftY, 0.0f));
                model = glm::rotate(model, time * 0.8f + currentPos.x, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            }
            case ShapeType::CYLINDER: {
                // Blue Flying Saucer: continuous turntable spin + 18-degree tilt showing off equatorial collar
                float spinAngle = time * 1.8f + currentPos.x * 2.0f + currentPos.z * 1.8f;
                model = glm::rotate(model, glm::radians(18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, spinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            }
            case ShapeType::CUBE: {
                // Green Square Pillow: gentle cushion wobble tilt
                float wobble = 0.10f * std::sin(time * 3.0f + currentPos.x * 1.3f + currentPos.z * 0.9f);
                model = glm::rotate(model, wobble, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            }
            case ShapeType::DIAMOND: {
                // Orange Water Drop: floating bob + swaying pendulum tilt
                float bobY = 0.05f * std::sin(time * 2.2f + currentPos.x * 1.5f + currentPos.z * 1.2f);
                model = glm::translate(model, glm::vec3(0.0f, bobY, 0.0f));
                float sway = 0.12f * std::sin(time * 2.0f + currentPos.x);
                model = glm::rotate(model, sway, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            }
            default:
                break;
        }
    }

    // Golden candies spin smoothly with elegant metallic luster (no scale pulsation)
    if (isGolden && !isSelected && !isAnimating) {
        model = glm::rotate(model, time * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (rotationY != 0.0f) {
        model = glm::rotate(model, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    float currentScale = scale;

    // Red Bent Cylinder breathe/pulse is applied as scale (unique idle motion)
    if (type == ShapeType::SPHERE && !isSelected && !isAnimating) {
        float breathe = 1.0f + 0.06f * std::sin(time * 4.0f + currentPos.x * 1.1f + currentPos.z * 1.4f);
        currentScale *= breathe;
    }

    switch (type) {
        case ShapeType::SPHERE:
            return glm::scale(model, glm::vec3(currentScale * 1.0f));
        case ShapeType::CYLINDER:
            return glm::scale(model, glm::vec3(currentScale * 1.0f));
        case ShapeType::CUBE:
            return glm::scale(model, glm::vec3(currentScale * 1.0f));
        case ShapeType::DIAMOND:
            return glm::scale(model, glm::vec3(currentScale * 1.0f));
        default:
            return model;
    }
}

void ShapeTile::draw(const Shader& shader, const Mesh& bentCylMesh, const Mesh& saucerMesh,
                    const Mesh& pillowMesh, const Mesh& teardropMesh, bool isSelected, float time) const {
    if (type == ShapeType::NONE || (scale <= 0.001f && !isPopping)) return;

    glm::mat4 model = getModelMatrix(time, isSelected);

    glm::vec4 color = getShapeColor(type);
    if (isGolden) {
        // 24K Rich Deep Amber Gold Bullion (Precious Gold Bonus Candy)
        color = glm::vec4(1.00f, 0.58f, 0.04f, 1.0f);
    }
    shader.setVec4("objectColor", color);

    shader.setBool("isTray", false);
    shader.setBool("isGolden", isGolden);
    shader.setMat4("model", model);

    // Draw corresponding Candy Crush confectionery primitive
    switch (type) {
        case ShapeType::SPHERE:   bentCylMesh.draw(); break;    // Red Bent Cylinder
        case ShapeType::CYLINDER: saucerMesh.draw(); break;     // Blue Flying Saucer
        case ShapeType::CUBE:     pillowMesh.draw(); break;     // Green Square Pillow
        case ShapeType::DIAMOND:  teardropMesh.draw(); break;   // Orange Teardrop
        default: break;
    }
}
