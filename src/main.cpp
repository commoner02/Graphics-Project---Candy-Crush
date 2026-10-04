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
                                std::to_string(g_board.selectedCoord.col) + ", Level " +
                                std::to_string(g_board.selectedCoord.level) + ")";
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
              << "       3D GEOMETRIC SHAPE-MATCH: 3-RING COLOSSEUM EDITION\n"
              << "==================================================================\n"
              << " ARCHITECTURE (160 PROCEDURAL SHAPES):\n"
              << "   - Layer 2 (Summit) : 4x4 Full Board     (16 Shapes at Y=6.15)\n"
              << "   - Layer 1 (Middle) : 6x6 Wall of 3 Rings (60 Shapes, 20/ring, Levels 0..2)\n"
              << "   - Layer 0 (Base)   : 8x8 Wall of 3 Rings (84 Shapes, 28/ring, Levels 0..2)\n"
              << " MATCH-3 RULES ON 3-STORY WALLS:\n"
              << "   - 4-Way Swapping   : Swap Left/Right around ring, or Up/Down on wall!\n"
              << "   - Horizontal Match : 3-in-a-row around perimeter at same level.\n"
              << "   - Vertical Match   : 3-in-a-row column straight up/down the wall.\n"
              << "   - Wall Gravity     : Shapes drop down -Y towards floor; new spawn at top.\n"
              << " SHAPES & VIBRANT PALETTE:\n"
              << "   [1] UV Sphere     (Ruby Red)      : 16 rings x 20 sectors = 640 Triangles\n"
              << "   [2] Oct. Cylinder (Emerald Green) : 8 sectors + 2 fan caps = 32 Triangles\n"
              << "   [3] Unit Cube     (Topaz Gold)    : 6 faces x 2 triangles = 12 Triangles\n"
              << "   [4] Bipyramid     (Electric Cyan) : 8 equatorial facets    = 16 Triangles\n"
              << " CONTROLS:\n"
              << "   - Left Mouse Click   : Select Shape & Click adjacent neighbor to SWAP\n"
              << "   - [E] Key            : Exploded Layer View (Separates Tiers along Y)\n"
              << "   - [W] Key            : Toggle Wireframe / Solid Architectural Shading\n"
              << "   - [P] Key            : Toggle Perspective / Orthographic Blueprint View\n"
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
                          "3D COLOSSEUM (3-RING WALLS) | Score: %d | Combo: %dx | [E] Explode | [W] Wire/Solid",
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
