#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

class Camera {
public:
    glm::vec3 target = glm::vec3(0.0f, 3.2f, 0.0f);
    float distance = 17.5f;
    float yaw = 45.0f;       // Horizontal orbit angle (degrees)
    float pitch = 26.0f;     // Vertical elevation angle (degrees)
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

