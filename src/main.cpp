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
#include "HUD.h"

// -----------------------------------------------------------------------------
// Global Application State
// -----------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

// Shading & Rendering state: Solid 3D by default with Phong specular
bool g_wireframe = false;
bool g_orthographic = false;

// Shading Mode: 0 = Flat/Clay, 1 = Gouraud (per-vertex), 2 = Phong (per-fragment specular)
int  g_shadingMode = 2;
bool g_showNormalMap = false; // Normal Vector Visualization (RGB = Normal * 0.5 + 0.5)
bool g_brushEffect   = true;  // Artisan Hand-Brushed Glaze & Anisotropic Silky Ribbon

HUD g_hud;


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
            std::cout << "[Mode] " << (g_wireframe ? "PURE WIREFRAME (GL_LINE)" : "SOLID SHADED (GL_FILL)") << std::endl;
        } else if (key == GLFW_KEY_G) {
            g_showNormalMap = false;
            g_shadingMode = (g_shadingMode + 1) % 3;
            if (g_wireframe) g_wireframe = false;
            const char* sNames[] = {
                "1. FLAT SHADING (Basic Ambient + Diffuse)",
                "2. GOURAUD SHADING (Per-Vertex Lighting Calculation)",
                "3. PHONG SHADING (Per-Fragment Blinn-Phong Specular Highlight)"
            };
            std::cout << "[Shading Model: Key G] Switched to: " << sNames[g_shadingMode] << std::endl;
        } else if (key == GLFW_KEY_N) {
            g_showNormalMap = !g_showNormalMap;
            if (g_showNormalMap && g_wireframe) g_wireframe = false;
            std::cout << "[Shading Model: Key N] Surface Normal Map: "
                      << (g_showNormalMap ? "ON (RGB = Normal * 0.5 + 0.5)" : "OFF (Standard Shading)") << std::endl;
        } else if (key == GLFW_KEY_B) {
            g_board.cycleTheme();
            std::cout << "[Theme: Key B] Pyramid & Background Theme: " << g_board.getThemeName() << std::endl;
        } else if (key == GLFW_KEY_C) {
            g_brushEffect = !g_brushEffect;
            std::cout << "[Confectionery: Key C] Artisan Hand-Brushed Glaze: "
                      << (g_brushEffect ? "ON (Silky Pulled-Sugar Satin Ribbon + Bakery Glaze Swipe)" : "OFF (Smooth Glossy Jelly)") << std::endl;
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

        // Clickable HUD Top-Right Controls Check
        int ctrl = HUD::checkControlClick(mx, my, w, h);
        if (ctrl > 0) {
            if (ctrl == 1) { // [G] Shading
                g_showNormalMap = false;
                g_shadingMode = (g_shadingMode + 1) % 3;
                if (g_wireframe) g_wireframe = false;
                std::cout << "[HUD Click: Key G] Shading Mode changed to: " << g_shadingMode << std::endl;
            } else if (ctrl == 2) { // [B] Theme
                g_board.cycleTheme();
                std::cout << "[HUD Click: Key B] Theme: " << g_board.getThemeName() << std::endl;
            } else if (ctrl == 3) { // [W] Wireframe
                g_wireframe = !g_wireframe;
                std::cout << "[HUD Click: Key W] Wireframe: " << (g_wireframe ? "ON" : "OFF") << std::endl;
            } else if (ctrl == 4) { // [P] Projection
                g_orthographic = !g_orthographic;
                std::cout << "[HUD Click: Key P] Projection: " << (g_orthographic ? "ORTHO" : "PERSP") << std::endl;
            }
            return;
        }

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

        std::cout << "[3D Raycasting Unproject] Screen (" << mx << ", " << my << ") -> Ray Origin: [ "
                  << ray.origin.x << ", " << ray.origin.y << ", " << ray.origin.z << " ], Dir: [ "
                  << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << " ]" << std::endl;

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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Candy Crush Pyramid", nullptr, nullptr);
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
        std::cerr << "Failed to build main shader program!" << std::endl;
        return -1;
    }

    // Initialize 2D Vector & Digital HUD
    if (!g_hud.init()) {
        std::cerr << "[HUD] Warning: Failed to build HUD shader program!" << std::endl;
    }


    // 5. Generate Procedural Geometric Meshes (All pure GL_TRIANGLES)
    Mesh trayMesh     = Primitives::createCube();                            // Foundation Slabs & Pillars (12 tris)
    Mesh bentCylMesh  = Primitives::createBentCylinder();                    // Red Bent Cylinder with Rounded Caps (1560 tris)
    Mesh saucerMesh   = Primitives::createFlyingSaucer(24, 24, 0.32f, 0.14f);// Blue Flying Saucer / Saturn Drop (1152 tris)
    Mesh pillowMesh   = Primitives::createPillow(20, 28, 0.35f, 0.18f);      // Green Square Pillow (1120 tris)
    Mesh teardropMesh = Primitives::createTeardrop(24, 24, 0.36f, 0.70f);    // Orange Teardrop (1152 tris)

    // Print Console Instructions
    std::cout << "\n==================================================================\n"
              << "                    3D CANDY CRUSH PYRAMID\n"
              << "==================================================================\n"
              << " ARCHITECTURE (160 PROCEDURAL CANDIES):\n"
              << "   - Summit (Layer 2) : 4x4 Full Board     (16 Candies at Y=6.15)\n"
              << "   - Middle (Layer 1) : 6x6 Wall of 3 Rings (60 Candies, 20/ring, Levels 0..2)\n"
              << "   - Base   (Layer 0) : 8x8 Wall of 3 Rings (84 Candies, 28/ring, Levels 0..2)\n"
              << " ICONIC CANDY CRUSH SWEETS:\n"
              << "   - Red Bent Cyl     : Cherry Bent Cylinder (Curved like a jelly bean, smooth Y-spin + breathe)\n"
              << "   - Blue Saucer      : Blue Flying Saucer with Collar (28 rad/s turntable spin)\n"
              << "   - Green Pillow     : Lime Puffed Square Pillow (3D chaotic tumble)\n"
              << "   - Yellow Water Drop: Candied Lemon Water Drop (Upward piriform droplet, floating bob + droplet sway)\n"
              << "   - 24K Bronze Gold  : Rare Bonus Candy (+500 pts, Supernova burst)\n"
              << " MATCH-3 RULES ON 3-STORY WALLS:\n"
              << "   - 4-Way Swapping   : Swap Left/Right around ring, or Up/Down on wall!\n"
              << "   - Golden Bonus     : Rare 24K Gold Shapes award +500 EXTRA BONUS POINTS!\n"
              << "   - Wall Gravity     : Shapes drop down -Y towards floor; new spawn at top.\n"
              << " SHADING & LIGHTING (ACADEMIC SHOWCASE):\n"
              << "   - [G] Key            : Cycle Shading: Flat -> Gouraud (Per-Vertex) -> Phong (Per-Fragment)\n"
              << "   - [C] Key            : Toggle Artisan Hand-Brushed Glaze (Silky Ribbon Sheen)\n"
              << "   - [N] Key            : Toggle Surface Normal Vector Map (RGB = Normal * 0.5 + 0.5)\n"
              << "   - [W] Key            : Toggle Wireframe / Solid Shading Mode\n"
              << " CONTROLS:\n"
              << "   - Left Mouse Click   : Select & Swap (Shows 3D Unprojected Ray in World Space)\n"
              << "   - [E] Key            : Exploded Layer View (Separates Tiers along Y)\n"
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

        // Update Dynamic Window Title with Shading Mode & Combo Status
        char title[256];
        const char* modeName = (g_showNormalMap ? "NORMAL MAP" :
                               (g_shadingMode == 1 ? "GOURAUD" :
                               (g_shadingMode == 0 ? "FLAT" : (g_wireframe ? "WIREFRAME" : "PHONG"))));

        if (g_board.state == GameState::GAME_OVER) {
            std::snprintf(title, sizeof(title),
                          "GAME OVER! | Final Score: %d | %s | [R] Restart",
                          g_board.score, g_board.getThemeName().c_str());
        } else if (g_board.combo > 1) {
            std::snprintf(title, sizeof(title),
                          "\xf0\x9f\x94\xa5 COMBO x%d! | Score: %d | Moves: %d | %s | %s",
                          g_board.combo, g_board.score, g_board.movesLeft, modeName, g_board.getThemeName().c_str());
        } else {
            std::snprintf(title, sizeof(title),
                          "3D CANDY PYRAMID | Score: %d | Moves: %d | %s | %s",
                          g_board.score, g_board.movesLeft, modeName, g_board.getThemeName().c_str());
        }
        glfwSetWindowTitle(window, title);

        // Clear screen with active theme background color
        glm::vec4 bg = g_board.getBackgroundColor();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
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

        // -------------------------------------------------------------
        // RASTERIZATION PIPELINE (GL_TRIANGLES)
        // -------------------------------------------------------------
        // Toggle Polygon Mode
        if (g_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE); // See through all triangles in wireframe mode
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }

        shader.use();
        shader.setBool("isWireframe", g_wireframe);
        shader.setBool("isBrushed", g_brushEffect);
        shader.setInt("shadingMode", g_showNormalMap ? 3 : g_shadingMode);
        shader.setVec3("viewPos", g_camera.getPosition());
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // Draw Stepped Pyramid Trays and Candy Crush Sweets
        g_board.draw(shader, trayMesh, bentCylMesh, saucerMesh, pillowMesh, teardropMesh, static_cast<float>(currentTime));


        // -------------------------------------------------------------
        // IN-GAME ARCADE HUD OVERLAY
        // -------------------------------------------------------------
        const ShapeTile* selectedTile = g_board.selectedCoord.isValid() ? g_board.getTile(g_board.selectedCoord) : nullptr;

        // Convert Board popups to HUD format
        std::vector<ScorePopupData> hudPopups;
        for (const auto& p : g_board.scorePopups) {
            hudPopups.push_back({p.x, p.y, p.points, p.timer, p.maxTime});
        }

        bool isGameOver = (g_board.state == GameState::GAME_OVER);
        g_hud.draw(width, height, g_board.score, g_board.combo,
                   g_board.movesLeft, isGameOver,
                   g_shadingMode, g_showNormalMap, g_wireframe, g_orthographic,
                   static_cast<int>(g_board.theme),
                   selectedTile, static_cast<float>(currentTime), hudPopups);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup Resources

    g_hud.destroy();
    trayMesh.destroy();
    bentCylMesh.destroy();
    saucerMesh.destroy();
    pillowMesh.destroy();
    teardropMesh.destroy();
    shader.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
