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
    target = glm::vec3(0.0f, 3.2f, 0.0f);
    distance = 17.5f;
    yaw = 45.0f;
    pitch = 26.0f;
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

