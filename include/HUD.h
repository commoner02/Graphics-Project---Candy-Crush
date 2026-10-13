#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"
#include "Shape.h"

struct ScorePopupData {
    float x, y;
    int points;
    float timer;
    float maxTime;
};

class HUD {
public:
    HUD() = default;
    ~HUD();

    bool init();
    void draw(int screenWidth, int screenHeight, int score, int combo,
              int movesLeft, bool isGameOver,
              int shadingMode, bool isNormalMap, bool isWireframe, bool isOrtho,
              int theme,
              const ShapeTile* selectedTile, float time,
              const std::vector<struct ScorePopupData>& popups);
    void destroy();

    // Checks if mouse click is inside top-right control buttons:
    // Returns: 1 = G (Shading), 2 = B (Theme), 3 = W (Wireframe), 4 = P (Projection), 0 = None
    static int checkControlClick(double mx, double my, int screenWidth, int screenHeight);

private:
    Shader hudShader;
    GLuint vao = 0;
    GLuint vbo = 0;

    struct Vertex2D {
        float x, y;
        float r, g, b, a;
    };
    std::vector<Vertex2D> batchVertices;

    void addRect(float x, float y, float w, float h, const glm::vec4& color);
    void addRectOutline(float x, float y, float w, float h, float thickness, const glm::vec4& color);
    void addSegment(float x1, float y1, float x2, float y2, float thickness, const glm::vec4& color);
    void add7SegmentDigit(float x, float y, float w, float h, float thickness, int digit, const glm::vec4& color);
    void add7SegmentNumber(float x, float y, float digitW, float digitH, float spacing, float thickness, int number, int minDigits, const glm::vec4& color);
    void addVectorChar(float x, float y, float w, float h, float thickness, char c, const glm::vec4& color);
    void addVectorText(float x, float y, float charW, float charH, float spacing, float thickness, const std::string& text, const glm::vec4& color);
    void flush();
};
