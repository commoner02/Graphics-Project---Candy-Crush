#include "HUD.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

HUD::~HUD() {
    destroy();
}

bool HUD::init() {
    if (!hudShader.init("shaders/hud.vert", "shaders/hud.frag")) {
        std::cerr << "[HUD] Failed to compile HUD shaders!" << std::endl;
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * 16384, nullptr, GL_DYNAMIC_DRAW);

    // Location 0: 2D Position (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glEnableVertexAttribArray(0);

    // Location 1: Color (vec4)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return true;
}

void HUD::destroy() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    hudShader.destroy();
}

void HUD::addRect(float x, float y, float w, float h, const glm::vec4& color) {
    Vertex2D v1{x, y, color.r, color.g, color.b, color.a};
    Vertex2D v2{x, y + h, color.r, color.g, color.b, color.a};
    Vertex2D v3{x + w, y + h, color.r, color.g, color.b, color.a};
    Vertex2D v4{x + w, y, color.r, color.g, color.b, color.a};

    // First triangle (v1, v2, v3)
    batchVertices.push_back(v1);
    batchVertices.push_back(v2);
    batchVertices.push_back(v3);

    // Second triangle (v1, v3, v4)
    batchVertices.push_back(v1);
    batchVertices.push_back(v3);
    batchVertices.push_back(v4);
}

void HUD::addRectOutline(float x, float y, float w, float h, float thickness, const glm::vec4& color) {
    addRect(x, y, w, thickness, color);                         // Top
    addRect(x, y + h - thickness, w, thickness, color);         // Bottom
    addRect(x, y + thickness, thickness, h - 2 * thickness, color); // Left
    addRect(x + w - thickness, y + thickness, thickness, h - 2 * thickness, color); // Right
}

void HUD::addSegment(float x1, float y1, float x2, float y2, float thickness, const glm::vec4& color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-4f) return;

    float nx = (-dy / len) * (thickness * 0.5f);
    float ny = (dx / len) * (thickness * 0.5f);

    Vertex2D v1{x1 + nx, y1 + ny, color.r, color.g, color.b, color.a};
    Vertex2D v2{x1 - nx, y1 - ny, color.r, color.g, color.b, color.a};
    Vertex2D v3{x2 - nx, y2 - ny, color.r, color.g, color.b, color.a};
    Vertex2D v4{x2 + nx, y2 + ny, color.r, color.g, color.b, color.a};

    // CCW triangle winding for 2D screen space
    batchVertices.push_back(v1);
    batchVertices.push_back(v3);
    batchVertices.push_back(v2);
    batchVertices.push_back(v1);
    batchVertices.push_back(v4);
    batchVertices.push_back(v3);
}

void HUD::add7SegmentDigit(float x, float y, float w, float h, float thickness, int digit, const glm::vec4& color) {
    if (digit < 0 || digit > 9) digit = 0;

    static const uint8_t patterns[10] = {
        0b0111111, // 0: a, b, c, d, e, f
        0b0000110, // 1: b, c
        0b1011011, // 2: a, b, d, e, g
        0b1001111, // 3: a, b, c, d, g
        0b1100110, // 4: b, c, f, g
        0b1101101, // 5: a, c, d, f, g
        0b1111101, // 6: a, c, d, e, f, g
        0b0000111, // 7: a, b, c
        0b1111111, // 8: a, b, c, d, e, f, g
        0b1101111  // 9: a, b, c, d, f, g
    };

    uint8_t p = patterns[digit];
    float halfH = h * 0.5f;

    // Segment a (Top)
    if (p & (1 << 0)) addRect(x + thickness, y, w - 2 * thickness, thickness, color);
    // Segment b (Top-Right)
    if (p & (1 << 1)) addRect(x + w - thickness, y + thickness, thickness, halfH - thickness, color);
    // Segment c (Bottom-Right)
    if (p & (1 << 2)) addRect(x + w - thickness, y + halfH, thickness, halfH - thickness, color);
    // Segment d (Bottom)
    if (p & (1 << 3)) addRect(x + thickness, y + h - thickness, w - 2 * thickness, thickness, color);
    // Segment e (Bottom-Left)
    if (p & (1 << 4)) addRect(x, y + halfH, thickness, halfH - thickness, color);
    // Segment f (Top-Left)
    if (p & (1 << 5)) addRect(x, y + thickness, thickness, halfH - thickness, color);
    // Segment g (Middle)
    if (p & (1 << 6)) addRect(x + thickness, y + halfH - thickness * 0.5f, w - 2 * thickness, thickness, color);
}

void HUD::add7SegmentNumber(float x, float y, float digitW, float digitH, float spacing, float thickness, int number, int minDigits, const glm::vec4& color) {
    std::string s = std::to_string(number);
    while (static_cast<int>(s.length()) < minDigits) {
        s = "0" + s;
    }

    float curX = x;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            add7SegmentDigit(curX, y, digitW, digitH, thickness, c - '0', color);
        }
        curX += digitW + spacing;
    }
}

void HUD::addVectorChar(float x, float y, float w, float h, float thickness, char c, const glm::vec4& color) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');

    float x0 = x, x1 = x + w * 0.5f, x2 = x + w;
    float y0 = y, y1 = y + h * 0.5f, y2 = y + h;

    switch (c) {
        case 'A':
            addSegment(x0, y2, x0, y1, thickness, color);
            addSegment(x0, y1, x1, y0, thickness, color);
            addSegment(x1, y0, x2, y1, thickness, color);
            addSegment(x2, y1, x2, y2, thickness, color);
            addSegment(x0, y1, x2, y1, thickness, color);
            break;
        case 'B': {
            float yMidTop = y0 + (y1 - y0) * 0.5f;
            float yMidBot = y1 + (y2 - y1) * 0.5f;
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x1, y0, thickness, color);
            addSegment(x0, y1, x1, y1, thickness, color);
            addSegment(x0, y2, x1, y2, thickness, color);
            addSegment(x1, y0, x2, yMidTop, thickness, color);
            addSegment(x2, yMidTop, x1, y1, thickness, color);
            addSegment(x1, y1, x2, yMidBot, thickness, color);
            addSegment(x2, yMidBot, x1, y2, thickness, color);
            break;
        }
        case 'C':
            addSegment(x2, y0, x0, y0, thickness, color);
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case 'D':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x1, y0, thickness, color);
            addSegment(x0, y2, x1, y2, thickness, color);
            addSegment(x1, y0, x2, y1, thickness, color);
            addSegment(x2, y1, x1, y2, thickness, color);
            break;
        case 'E':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x0, y1, x1 + thickness, y1, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case 'F':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x0, y1, x1, y1, thickness, color);
            break;
        case 'G':
            addSegment(x2, y0, x0, y0, thickness, color);
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            addSegment(x2, y2, x2, y1, thickness, color);
            addSegment(x1, y1, x2, y1, thickness, color);
            break;
        case 'H':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x2, y0, x2, y2, thickness, color);
            addSegment(x0, y1, x2, y1, thickness, color);
            break;
        case 'I':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x1, y0, x1, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case 'J':
            addSegment(x2, y0, x2, y2, thickness, color);
            addSegment(x2, y2, x0, y2, thickness, color);
            addSegment(x0, y2, x0, y1, thickness, color);
            break;
        case 'K':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y1, x2, y0, thickness, color);
            addSegment(x0, y1, x2, y2, thickness, color);
            break;
        case 'L':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case 'M':
            addSegment(x0, y2, x0, y0, thickness, color);
            addSegment(x0, y0, x1, y1, thickness, color);
            addSegment(x1, y1, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y2, thickness, color);
            break;
        case 'N':
            addSegment(x0, y2, x0, y0, thickness, color);
            addSegment(x0, y0, x2, y2, thickness, color);
            addSegment(x2, y2, x2, y0, thickness, color);
            break;
        case 'O':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y2, thickness, color);
            addSegment(x2, y2, x0, y2, thickness, color);
            addSegment(x0, y2, x0, y0, thickness, color);
            break;
        case 'P':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y1, thickness, color);
            addSegment(x2, y1, x0, y1, thickness, color);
            break;
        case 'Q':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y2, thickness, color);
            addSegment(x2, y2, x0, y2, thickness, color);
            addSegment(x0, y2, x0, y0, thickness, color);
            addSegment(x1, y1, x2, y2, thickness, color);
            break;
        case 'R':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y1, thickness, color);
            addSegment(x2, y1, x0, y1, thickness, color);
            addSegment(x0, y1, x2, y2, thickness, color);
            break;
        case 'S':
            addSegment(x2, y0, x0, y0, thickness, color);
            addSegment(x0, y0, x0, y1, thickness, color);
            addSegment(x0, y1, x2, y1, thickness, color);
            addSegment(x2, y1, x2, y2, thickness, color);
            addSegment(x2, y2, x0, y2, thickness, color);
            break;
        case 'T':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x1, y0, x1, y2, thickness, color);
            break;
        case 'U':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            addSegment(x2, y2, x2, y0, thickness, color);
            break;
        case 'V':
            addSegment(x0, y0, x1, y2, thickness, color);
            addSegment(x1, y2, x2, y0, thickness, color);
            break;
        case 'W':
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x1, y1, thickness, color);
            addSegment(x1, y1, x2, y2, thickness, color);
            addSegment(x2, y2, x2, y0, thickness, color);
            break;
        case 'X':
            addSegment(x0, y0, x2, y2, thickness, color);
            addSegment(x0, y2, x2, y0, thickness, color);
            break;
        case 'Y':
            addSegment(x0, y0, x1, y1, thickness, color);
            addSegment(x2, y0, x1, y1, thickness, color);
            addSegment(x1, y1, x1, y2, thickness, color);
            break;
        case 'Z':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            add7SegmentDigit(x, y, w, h, thickness, c - '0', color);
            break;
        case ':':
            addRect(x1 - thickness, y0 + h * 0.25f, thickness * 2.0f, thickness * 2.0f, color);
            addRect(x1 - thickness, y0 + h * 0.70f, thickness * 2.0f, thickness * 2.0f, color);
            break;
        case '-':
            addSegment(x0, y1, x2, y1, thickness, color);
            break;
        case '[':
            addSegment(x2, y0, x0, y0, thickness, color);
            addSegment(x0, y0, x0, y2, thickness, color);
            addSegment(x0, y2, x2, y2, thickness, color);
            break;
        case ']':
            addSegment(x0, y0, x2, y0, thickness, color);
            addSegment(x2, y0, x2, y2, thickness, color);
            addSegment(x2, y2, x0, y2, thickness, color);
            break;
        case '|':
            addSegment(x1, y0, x1, y2, thickness, color);
            break;
        case '/':
            addSegment(x0, y2, x2, y0, thickness, color);
            break;
        case 'x':
            addSegment(x0 + w * 0.2f, y1 - h * 0.3f, x2 - w * 0.2f, y2, thickness, color);
            addSegment(x0 + w * 0.2f, y2, x2 - w * 0.2f, y1 - h * 0.3f, thickness, color);
            break;
        default:
            break;
    }
}

void HUD::addVectorText(float x, float y, float charW, float charH, float spacing, float thickness, const std::string& text, const glm::vec4& color) {
    float curX = x;
    for (char c : text) {
        if (c != ' ') {
            addVectorChar(curX, y, charW, charH, thickness, c, color);
        }
        curX += charW + spacing;
    }
}

void HUD::flush() {
    if (batchVertices.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, batchVertices.size() * sizeof(Vertex2D), batchVertices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(batchVertices.size()));
    glBindVertexArray(0);

    batchVertices.clear();
}

int HUD::checkControlClick(double mx, double my, int screenWidth, int screenHeight) {
    (void)screenHeight;
    float sW = 280.0f, sH = 142.0f;
    float sX = screenWidth - sW - 20.0f, sY = 20.0f;

    if (mx < sX || mx > sX + sW || my < sY || my > sY + sH) return 0;

    // Row 0: [G] Shading
    if (my >= sY + 22.0f && my < sY + 50.0f) return 1;
    // Row 1: [B] Theme
    if (my >= sY + 50.0f && my < sY + 78.0f) return 2;
    // Row 2: [W] Wireframe
    if (my >= sY + 78.0f && my < sY + 106.0f) return 3;
    // Row 3: [P] Projection
    if (my >= sY + 106.0f && my <= sY + 138.0f) return 4;

    return 0;
}

void HUD::draw(int screenWidth, int screenHeight, int score, int combo,
              int movesLeft, bool isGameOver,
              int shadingMode, bool isNormalMap, bool isWireframe, bool isOrtho,
              int theme,
              const ShapeTile* selectedTile, float time,
              const std::vector<ScorePopupData>& popups) {
    if (screenWidth <= 0 || screenHeight <= 0) return;

    // Save OpenGL states
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint prevPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // Essential: Prevents GPU culling of 2D HUD vector characters
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    hudShader.use();
    glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
    hudShader.setMat4("projection", proj);

    batchVertices.clear();

    // -------------------------------------------------------------------------
    // 1. TOP-LEFT: ARCADE SCORE & COMBO PANEL
    // -------------------------------------------------------------------------
    float pW = 310.0f, pH = 115.0f;
    float pX = 20.0f, pY = 20.0f;

    // Dark semi-transparent card background
    addRect(pX, pY, pW, pH, glm::vec4(0.07f, 0.09f, 0.14f, 0.88f));
    // Glowing neon cyan border
    addRectOutline(pX, pY, pW, pH, 2.0f, glm::vec4(0.00f, 0.90f, 1.00f, 0.85f));
    // Header accent bar
    addRect(pX, pY, pW, 4.0f, glm::vec4(0.00f, 0.90f, 1.00f, 1.00f));

    // "SCORE" text label
    addVectorText(pX + 16.0f, pY + 16.0f, 9.0f, 14.0f, 4.0f, 2.0f, "SCORE", glm::vec4(0.60f, 0.75f, 0.95f, 1.0f));

    // 7-Segment Digital Score Display (Gold LED font)
    glm::vec4 goldLedColor = glm::vec4(1.00f, 0.88f, 0.10f, 1.0f);
    add7SegmentNumber(pX + 85.0f, pY + 12.0f, 15.0f, 24.0f, 6.0f, 2.5f, score, 6, goldLedColor);

    // Combo Section
    if (combo > 1) {
        // Glowing orange/crimson combo banner
        float pulse = 0.5f + 0.5f * std::sin(time * 10.0f);
        glm::vec4 comboColor = glm::mix(glm::vec4(1.00f, 0.35f, 0.05f, 1.0f), glm::vec4(1.00f, 0.80f, 0.10f, 1.0f), pulse);

        std::string comboStr = "COMBO X" + std::to_string(combo) + " CASCADE";
        addVectorText(pX + 16.0f, pY + 52.0f, 10.0f, 16.0f, 4.0f, 2.5f, comboStr, comboColor);

        // Animated Multiplier Power Meter
        float meterMaxW = pW - 32.0f;
        float meterFill = std::min(1.0f, (combo - 1) * 0.25f);
        addRect(pX + 16.0f, pY + 80.0f, meterMaxW, 14.0f, glm::vec4(0.12f, 0.15f, 0.22f, 0.9f));
        addRect(pX + 16.0f, pY + 80.0f, meterMaxW * meterFill, 14.0f, comboColor);
        addRectOutline(pX + 16.0f, pY + 80.0f, meterMaxW, 14.0f, 1.5f, glm::vec4(1.00f, 0.80f, 0.20f, 0.9f));
    } else {
        // Standard combo standby line
        addVectorText(pX + 16.0f, pY + 54.0f, 8.0f, 12.0f, 3.0f, 1.8f, "COMBO READY", glm::vec4(0.40f, 0.55f, 0.70f, 0.9f));

        // Inactive power bar
        float meterMaxW = pW - 32.0f;
        addRect(pX + 16.0f, pY + 78.0f, meterMaxW, 10.0f, glm::vec4(0.12f, 0.15f, 0.22f, 0.8f));
        addRectOutline(pX + 16.0f, pY + 78.0f, meterMaxW, 10.0f, 1.0f, glm::vec4(0.25f, 0.35f, 0.50f, 0.8f));
    }

    // -------------------------------------------------------------------------
    // 1B. MOVES REMAINING PANEL (Below score panel)
    // -------------------------------------------------------------------------
    float mW = 180.0f, mH = 55.0f;
    float mX = pX, mY = pY + pH + 10.0f;

    addRect(mX, mY, mW, mH, glm::vec4(0.07f, 0.09f, 0.14f, 0.88f));

    // Color moves based on urgency
    glm::vec4 movesBorderColor = glm::vec4(0.35f, 0.55f, 0.95f, 0.85f);
    glm::vec4 movesNumColor = glm::vec4(0.80f, 0.90f, 1.00f, 1.0f);
    if (movesLeft <= 5) {
        float urgentPulse = 0.5f + 0.5f * std::sin(time * 8.0f);
        movesBorderColor = glm::mix(glm::vec4(1.0f, 0.15f, 0.15f, 0.9f), glm::vec4(1.0f, 0.5f, 0.1f, 0.9f), urgentPulse);
        movesNumColor = movesBorderColor;
    } else if (movesLeft <= 10) {
        movesBorderColor = glm::vec4(1.0f, 0.65f, 0.10f, 0.85f);
        movesNumColor = movesBorderColor;
    }

    addRectOutline(mX, mY, mW, mH, 2.0f, movesBorderColor);
    addRect(mX, mY, mW, 3.0f, movesBorderColor);

    addVectorText(mX + 12.0f, mY + 12.0f, 8.0f, 12.0f, 3.0f, 1.8f, "MOVES", glm::vec4(0.55f, 0.65f, 0.80f, 0.9f));
    add7SegmentNumber(mX + 85.0f, mY + 10.0f, 14.0f, 22.0f, 5.0f, 2.5f, std::max(0, movesLeft), 2, movesNumColor);

    // Moves progress bar
    float movesBarW = mW - 24.0f;
    float movesFill = static_cast<float>(std::max(0, movesLeft)) / 30.0f;
    addRect(mX + 12.0f, mY + 38.0f, movesBarW, 8.0f, glm::vec4(0.12f, 0.15f, 0.22f, 0.8f));
    addRect(mX + 12.0f, mY + 38.0f, movesBarW * movesFill, 8.0f, movesBorderColor);

    // -------------------------------------------------------------------------
    // 2. TOP-RIGHT: OPTION 2 COMPACT 4-ROW VISUAL CONTROLS PILL
    // -------------------------------------------------------------------------
    float sW = 280.0f, sH = 142.0f;
    float sX = screenWidth - sW - 20.0f, sY = 20.0f;

    // Dark sleek translucent card
    addRect(sX, sY, sW, sH, glm::vec4(0.06f, 0.08f, 0.13f, 0.92f));
    addRectOutline(sX, sY, sW, sH, 2.0f, glm::vec4(0.20f, 0.40f, 0.70f, 0.85f));

    // Card Header Bar
    addRect(sX, sY, sW, 22.0f, glm::vec4(0.10f, 0.14f, 0.24f, 0.98f));
    addRect(sX, sY + 22.0f, sW, 1.5f, glm::vec4(0.00f, 0.85f, 1.00f, 0.75f));
    addVectorText(sX + 14.0f, sY + 6.0f, 7.5f, 10.5f, 2.0f, 1.5f, "VISUAL CONTROLS", glm::vec4(0.00f, 0.90f, 1.00f, 1.0f));

    // Helper: Draw illuminated square keycap badge
    auto drawKeycap = [&](float kx, float ky, char keyChar, const glm::vec4& borderCol) {
        float kw = 20.0f, kh = 19.0f;
        addRect(kx, ky, kw, kh, glm::vec4(0.12f, 0.16f, 0.27f, 0.95f));
        addRectOutline(kx, ky, kw, kh, 1.5f, borderCol);
        addVectorChar(kx + 5.5f, ky + 4.0f, 7.5f, 11.5f, 2.0f, keyChar, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    };

    // Row 0: [ G ] SHADING
    float r0Y = sY + 29.0f;
    drawKeycap(sX + 12.0f, r0Y, 'G', glm::vec4(0.00f, 0.85f, 1.00f, 0.9f));
    addVectorText(sX + 38.0f, r0Y + 3.0f, 7.0f, 11.0f, 2.0f, 1.5f, "SHADING:", glm::vec4(0.60f, 0.72f, 0.88f, 1.0f));

    std::string sModeStr = "PHONG";
    glm::vec4 sModeCol = glm::vec4(0.18f, 1.00f, 0.55f, 1.0f); // Bright emerald for Phong
    if (isNormalMap) {
        sModeStr = "NORMALS";
        sModeCol = glm::vec4(1.00f, 0.25f, 0.85f, 1.0f); // Magenta
    } else if (shadingMode == 1) {
        sModeStr = "GOURAUD";
        sModeCol = glm::vec4(0.00f, 0.92f, 1.00f, 1.0f); // Cyan
    } else if (shadingMode == 0) {
        sModeStr = "FLAT";
        sModeCol = glm::vec4(1.00f, 0.75f, 0.15f, 1.0f); // Amber
    }
    addVectorText(sX + 125.0f, r0Y + 3.0f, 7.5f, 11.5f, 2.5f, 1.8f, sModeStr, sModeCol);

    // Row 1: [ B ] THEME
    float r1Y = sY + 56.0f;
    drawKeycap(sX + 12.0f, r1Y, 'B', glm::vec4(0.40f, 0.80f, 1.00f, 0.9f));
    addVectorText(sX + 38.0f, r1Y + 3.0f, 7.0f, 11.0f, 2.0f, 1.5f, "THEME  :", glm::vec4(0.60f, 0.72f, 0.88f, 1.0f));

    std::string themeStr = "SKY MARBLE";
    glm::vec4 themeCol = glm::vec4(0.40f, 0.82f, 1.00f, 1.0f); // Sky Blue
    if (theme == 1) {
        themeStr = "BAKERY";
        themeCol = glm::vec4(1.00f, 0.80f, 0.45f, 1.0f); // Warm Cream / Gold
    } else if (theme == 2) {
        themeStr = "MIDNIGHT";
        themeCol = glm::vec4(0.80f, 0.65f, 1.00f, 1.0f); // Midnight Lavender
    }
    addVectorText(sX + 125.0f, r1Y + 3.0f, 7.5f, 11.5f, 2.5f, 1.8f, themeStr, themeCol);

    // Row 2: [ W ] RENDER
    float r2Y = sY + 83.0f;
    drawKeycap(sX + 12.0f, r2Y, 'W', glm::vec4(1.00f, 0.75f, 0.20f, 0.9f));
    addVectorText(sX + 38.0f, r2Y + 3.0f, 7.0f, 11.0f, 2.0f, 1.5f, "RENDER :", glm::vec4(0.60f, 0.72f, 0.88f, 1.0f));

    std::string rModeStr = isWireframe ? "WIREFRAME" : "SOLID 3D";
    glm::vec4 rModeCol = isWireframe ? glm::vec4(0.20f, 0.90f, 1.00f, 1.0f) : glm::vec4(1.00f, 0.85f, 0.25f, 1.0f);
    addVectorText(sX + 125.0f, r2Y + 3.0f, 7.5f, 11.5f, 2.5f, 1.8f, rModeStr, rModeCol);

    // Row 3: [ P ] PROJECTION
    float r3Y = sY + 110.0f;
    drawKeycap(sX + 12.0f, r3Y, 'P', glm::vec4(0.85f, 0.60f, 1.00f, 0.9f));
    addVectorText(sX + 38.0f, r3Y + 3.0f, 7.0f, 11.0f, 2.0f, 1.5f, "CAMERA :", glm::vec4(0.60f, 0.72f, 0.88f, 1.0f));

    std::string pModeStr = isOrtho ? "ORTHO" : "PERSP 45D";
    glm::vec4 pModeCol = isOrtho ? glm::vec4(1.00f, 0.85f, 0.25f, 1.0f) : glm::vec4(0.65f, 0.78f, 0.95f, 1.0f);
    addVectorText(sX + 125.0f, r3Y + 3.0f, 7.5f, 11.5f, 2.5f, 1.8f, pModeStr, pModeCol);

    // -------------------------------------------------------------------------
    // 3. TARGET SELECTION CARD (When a tile is selected)
    // -------------------------------------------------------------------------
    if (!isGameOver && selectedTile != nullptr && selectedTile->type != ShapeType::NONE) {
        float cW = 280.0f, cH = 80.0f;
        float cX = 20.0f, cY = mY + mH + 10.0f;

        glm::vec4 shapeCol = getShapeColor(selectedTile->type);
        if (selectedTile->isGolden) shapeCol = glm::vec4(1.00f, 0.81f, 0.14f, 1.0f); // 24K Honey Candy Gold

        addRect(cX, cY, cW, cH, glm::vec4(0.07f, 0.09f, 0.14f, 0.90f));
        addRectOutline(cX, cY, cW, cH, 2.0f, shapeCol);

        // Vibrant Shape Icon Square
        addRect(cX + 12.0f, cY + 16.0f, 16.0f, 16.0f, shapeCol);
        addRectOutline(cX + 12.0f, cY + 16.0f, 16.0f, 16.0f, 1.5f, glm::vec4(1.0f));

        // Shape Name Header
        std::string nameHeader = selectedTile->isGolden ? "24K GOLD SHAPE" : getShapeName(selectedTile->type);
        addVectorText(cX + 36.0f, cY + 16.0f, 8.0f, 13.0f, 3.0f, 2.0f, nameHeader, shapeCol);

        // Coordinate Details
        std::string coordStr = "LAYER " + std::to_string(selectedTile->layer) + " | LVL " + std::to_string(selectedTile->level);
        addVectorText(cX + 16.0f, cY + 42.0f, 7.5f, 11.0f, 2.5f, 1.6f, coordStr, glm::vec4(0.70f, 0.80f, 0.90f, 0.95f));

        // Interaction Action Hint
        addVectorText(cX + 16.0f, cY + 60.0f, 7.0f, 9.5f, 2.0f, 1.4f, "CLICK ADJACENT TO SWAP", glm::vec4(0.00f, 1.00f, 0.55f, 0.95f));
    }

    // -------------------------------------------------------------------------
    // 4. FLOATING SCORE POPUPS
    // -------------------------------------------------------------------------
    for (const auto& popup : popups) {
        float alpha = std::min(1.0f, popup.timer / (popup.maxTime * 0.3f));
        float scaleFactor = 1.0f + (1.0f - popup.timer / popup.maxTime) * 0.3f;
        glm::vec4 popupColor = glm::vec4(1.0f, 0.92f, 0.20f, alpha);
        std::string popupStr = "+" + std::to_string(popup.points);
        addVectorText(popup.x, popup.y, 11.0f * scaleFactor, 18.0f * scaleFactor, 4.0f, 2.5f, popupStr, popupColor);
    }

    // -------------------------------------------------------------------------
    // 6. GAME OVER OVERLAY
    // -------------------------------------------------------------------------
    if (isGameOver) {
        // Full-screen dark overlay
        addRect(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight),
                glm::vec4(0.02f, 0.03f, 0.06f, 0.75f));

        // Central game over card
        float goW = 500.0f, goH = 200.0f;
        float goX = (screenWidth - goW) * 0.5f;
        float goY = (screenHeight - goH) * 0.5f - 30.0f;

        addRect(goX, goY, goW, goH, glm::vec4(0.08f, 0.10f, 0.16f, 0.95f));

        // Pulsing border
        float goPulse = 0.5f + 0.5f * std::sin(time * 4.0f);
        glm::vec4 goBorderColor = glm::mix(glm::vec4(1.0f, 0.2f, 0.2f, 0.9f), glm::vec4(1.0f, 0.6f, 0.1f, 0.9f), goPulse);
        addRectOutline(goX, goY, goW, goH, 3.0f, goBorderColor);
        addRect(goX, goY, goW, 5.0f, goBorderColor);

        // "GAME OVER" title
        addVectorText(goX + 105.0f, goY + 30.0f, 18.0f, 28.0f, 8.0f, 3.5f, "GAME OVER", goBorderColor);

        // Final score
        addVectorText(goX + 115.0f, goY + 80.0f, 10.0f, 16.0f, 4.0f, 2.5f, "FINAL SCORE", glm::vec4(0.65f, 0.75f, 0.90f, 0.95f));
        add7SegmentNumber(goX + 155.0f, goY + 110.0f, 18.0f, 30.0f, 7.0f, 3.0f, score, 6, goldLedColor);

        // Restart hint
        addVectorText(goX + 115.0f, goY + 160.0f, 9.0f, 14.0f, 4.0f, 2.0f, "PRESS [R] TO RESTART", glm::vec4(0.00f, 1.00f, 0.55f, 0.90f));
    }

    // Flush all 2D HUD batched vertices
    flush();

    // Restore OpenGL state
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (cullFaceEnabled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (!blendEnabled) glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
}
