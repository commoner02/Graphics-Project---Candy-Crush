#pragma once
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include "Shape.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

struct BoardCoord {
    int layer = -1; // 0 = Base (8x8), 1 = Middle (6x6), 2 = Top (4x4)
    int row = -1;
    int col = -1;
    int level = 0;  // 0, 1, 2 for vertical rings on walls; 0 for top board

    bool operator==(const BoardCoord& o) const {
        return layer == o.layer && row == o.row && col == o.col && level == o.level;
    }
    bool operator!=(const BoardCoord& o) const {
        return !(*this == o);
    }
    bool operator<(const BoardCoord& o) const {
        if (layer != o.layer) return layer < o.layer;
        if (level != o.level) return level < o.level;
        if (row != o.row) return row < o.row;
        return col < o.col;
    }
    bool isValid() const {
        return layer >= 0;
    }
};

enum class GameState {
    IDLE,
    SWAPPING,
    CHECKING_MATCH,
    POPPING,
    FALLING,
    REVERTING,
    GAME_OVER
};

enum class BoardTheme {
    FROSTED_MARBLE = 0,    // Theme 2: Pure Frosted White Sugar Marble (Clean & High Contrast)
    CHOCOLATE_BAKERY = 1,  // Theme 1: Milk Chocolate Core & Vanilla Biscuit Trays
    MIDNIGHT_ONYX = 2,     // Theme 3: Deep Obsidian & Royal Blue (Dark Arcade)
    COUNT = 3
};

class Board {
public:
    int score = 0;
    int combo = 0;
    int movesLeft = 30;
    static constexpr int MAX_MOVES = 30;
    GameState state = GameState::IDLE;

    // Floating score popups for HUD
    struct ScorePopup {
        float x, y;       // Screen-space position (set by Board at match location)
        int points;        // Points to display
        float timer;       // Lifetime remaining
        float maxTime;     // Total lifetime
    };
    std::vector<ScorePopup> scorePopups;

    BoardCoord selectedCoord;
    BoardCoord swapTargetCoord;

    bool isExploded = false;
    float explodeProgress = 0.0f;

    BoardTheme theme = BoardTheme::FROSTED_MARBLE; // Default: Theme 2 Frosted White Marble

    Board();

    void reset();
    void update(float deltaTime);
    void toggleExplode();
    void cycleTheme();
    glm::vec4 getBackgroundColor() const;
    std::string getThemeName() const;

    // 3D Raycasting selection
    BoardCoord pickTile(const Ray& ray) const;
    void onTileClicked(const BoardCoord& coord);

    // Draw full scene (pyramid trays, core columns, and shapes)
    void draw(const Shader& shader, const Mesh& trayMesh, const Mesh& bentCylMesh,
              const Mesh& saucerMesh, const Mesh& pillowMesh, const Mesh& teardropMesh, float time) const;

    static constexpr int NUM_LAYERS = 3;
    static constexpr int RINGS_PER_WALL = 3;
    static constexpr int L2_DIM = 4; // Top: 4x4 full board (16 shapes)
    static constexpr int L1_DIM = 6; // Middle: 6x6 wall of 3 rings (60 shapes)
    static constexpr int L0_DIM = 8; // Base: 8x8 wall of 3 rings (84 shapes)

    ShapeTile* getTile(const BoardCoord& coord);
    const ShapeTile* getTile(const BoardCoord& coord) const;
    bool isValidCoord(const BoardCoord& coord) const;
    bool isAdjacent(const BoardCoord& a, const BoardCoord& b) const;
    glm::vec3 getSlotWorldPos(int layer, int row, int col, int level = 0) const;

private:
    std::vector<ShapeTile> layer2;    // 4x4 full grid = 16 tiles (Top)
    std::vector<ShapeTile> layer1[3]; // 3 rings of 6x6 = 3 x 36 tiles (Middle Wall: 60 active)
    std::vector<ShapeTile> layer0[3]; // 3 rings of 8x8 = 3 x 64 tiles (Base Wall: 84 active)

    float stateTimer = 0.0f;
    std::set<BoardCoord> matchedCoords;

    void initLayerShapes();
    bool checkAndMarkMatches();
    void applyGravityAndRefill();
    void executeSwap(const BoardCoord& a, const BoardCoord& b, bool isRevert = false);
    ShapeType getRandomShapeType() const;
};
