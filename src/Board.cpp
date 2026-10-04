#include "Board.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <iostream>
#include <cmath>

Board::Board() {
    layer2.resize(L2_DIM * L2_DIM); // 4x4 = 16 tiles (Top)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        layer1[lvl].resize(L1_DIM * L1_DIM); // 6x6 = 36 tiles (Middle Wall: 20 per ring x 3 = 60 active)
        layer0[lvl].resize(L0_DIM * L0_DIM); // 8x8 = 64 tiles (Base Wall: 28 per ring x 3 = 84 active)
    }
    reset();
}

void Board::reset() {
    score = 0;
    combo = 0;
    state = GameState::IDLE;
    selectedCoord = BoardCoord();
    swapTargetCoord = BoardCoord();
    matchedCoords.clear();
    initLayerShapes();
}

ShapeType Board::getRandomShapeType() const {
    return static_cast<ShapeType>(std::rand() % static_cast<int>(ShapeType::COUNT));
}

void Board::toggleExplode() {
    isExploded = !isExploded;
    std::cout << "[View] Exploded Layer Mode " << (isExploded ? "ACTIVATED" : "DEACTIVATED") << std::endl;
    std::cout << "  [Transform: Explode Hierarchy] Tier Y-Offsets:\n"
              << "    Layer 2 (Top 4x4 Board)        : T( 0.000, " << (isExploded ? "+9.750" : "+6.150") << ", 0.000 )\n"
              << "    Layer 1 (Middle 6x6 Wall - 3R) : T( 0.000, " << (isExploded ? "+5.100" : "+3.300") << ", 0.000 )\n"
              << "    Layer 0 (Base 8x8 Wall - 3R)   : T( 0.000, +0.450, 0.000 )" << std::endl;
}

glm::vec3 Board::getSlotWorldPos(int layer, int row, int col, int level) const {
    float explodeOffset = 0.0f;
    if (layer == 1) explodeOffset = 1.80f * explodeProgress;
    else if (layer == 2) explodeOffset = 3.60f * explodeProgress;

    if (layer == 2) {
        // Top 4x4 horizontal board
        float x = (col - 1.5f) * 0.95f;
        float z = (row - 1.5f) * 0.95f;
        float y = 5.70f + 0.45f + explodeOffset;
        return glm::vec3(x, y, z);
    } else if (layer == 1) {
        // Middle 6x6 wall: 3 vertical rings (level 0, 1, 2)
        float x = (col - 2.5f) * 0.95f;
        float z = (row - 2.5f) * 0.95f;
        float y = 2.85f + 0.45f + level * 0.95f + explodeOffset;
        return glm::vec3(x, y, z);
    } else {
        // Base 8x8 wall: 3 vertical rings (level 0, 1, 2)
        float x = (col - 3.5f) * 0.95f;
        float z = (row - 3.5f) * 0.95f;
        float y = 0.00f + 0.45f + level * 0.95f;
        return glm::vec3(x, y, z);
    }
}

bool Board::isValidCoord(const BoardCoord& c) const {
    if (!c.isValid()) return false;
    if (c.layer == 2) {
        return (c.level == 0 && c.row >= 0 && c.row < L2_DIM && c.col >= 0 && c.col < L2_DIM);
    } else if (c.layer == 1) {
        if (c.level < 0 || c.level >= RINGS_PER_WALL) return false;
        if (c.row < 0 || c.row >= L1_DIM || c.col < 0 || c.col >= L1_DIM) return false;
        return (c.row == 0 || c.row == L1_DIM - 1 || c.col == 0 || c.col == L1_DIM - 1);
    } else if (c.layer == 0) {
        if (c.level < 0 || c.level >= RINGS_PER_WALL) return false;
        if (c.row < 0 || c.row >= L0_DIM || c.col < 0 || c.col >= L0_DIM) return false;
        return (c.row == 0 || c.row == L0_DIM - 1 || c.col == 0 || c.col == L0_DIM - 1);
    }
    return false;
}

ShapeTile* Board::getTile(const BoardCoord& c) {
    if (!isValidCoord(c)) return nullptr;
    if (c.layer == 2) return &layer2[c.row * L2_DIM + c.col];
    if (c.layer == 1) return &layer1[c.level][c.row * L1_DIM + c.col];
    if (c.layer == 0) return &layer0[c.level][c.row * L0_DIM + c.col];
    return nullptr;
}

const ShapeTile* Board::getTile(const BoardCoord& c) const {
    if (!isValidCoord(c)) return nullptr;
    if (c.layer == 2) return &layer2[c.row * L2_DIM + c.col];
    if (c.layer == 1) return &layer1[c.level][c.row * L1_DIM + c.col];
    if (c.layer == 0) return &layer0[c.level][c.row * L0_DIM + c.col];
    return nullptr;
}

bool Board::isAdjacent(const BoardCoord& a, const BoardCoord& b) const {
    if (!isValidCoord(a) || !isValidCoord(b)) return false;
    if (a.layer != b.layer) return false; // Swapping is within the same tier

    if (a.layer == 2) {
        int dr = std::abs(a.row - b.row);
        int dc = std::abs(a.col - b.col);
        return (dr + dc == 1);
    }

    // On vertical walls (Layer 1 and Layer 0):
    // 1. Vertical adjacency (swap Up / Down on the same perimeter slot)
    if (a.row == b.row && a.col == b.col && std::abs(a.level - b.level) == 1) {
        return true;
    }

    // 2. Horizontal adjacency (swap Left / Right on the same ring level)
    if (a.level == b.level) {
        int size = (a.layer == 1) ? L1_DIM : L0_DIM;
        auto getPerimeterIndex = [size](int r, int c) -> int {
            if (r == 0) return c;
            if (c == size - 1) return (size - 1) + r;
            if (r == size - 1) return (size - 1) + (size - 1) + ((size - 1) - c);
            if (c == 0) return (size - 1) * 3 + ((size - 1) - r);
            return -1;
        };

        int idxA = getPerimeterIndex(a.row, a.col);
        int idxB = getPerimeterIndex(b.row, b.col);
        if (idxA < 0 || idxB < 0) return false;

        int totalPerim = (size - 1) * 4;
        int diff = std::abs(idxA - idxB);
        return (diff == 1 || diff == totalPerim - 1);
    }

    return false;
}

void Board::initLayerShapes() {
    // 1. Top Layer 2 (4x4)
    for (int r = 0; r < L2_DIM; ++r) {
        for (int c = 0; c < L2_DIM; ++c) {
            ShapeTile* t = getTile({2, r, c, 0});
            t->layer = 2; t->row = r; t->col = c; t->level = 0;
            t->currentPos = getSlotWorldPos(2, r, c, 0);
            t->targetPos = t->currentPos;
            t->scale = 1.0f;
            t->isAnimating = false;
            t->type = getRandomShapeType();
        }
    }

    // 2. Middle Layer 1 (6x6 wall, 3 rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L1_DIM; ++r) {
            for (int c = 0; c < L1_DIM; ++c) {
                if (!isValidCoord({1, r, c, lvl})) continue;
                ShapeTile* t = getTile({1, r, c, lvl});
                t->layer = 1; t->row = r; t->col = c; t->level = lvl;
                t->currentPos = getSlotWorldPos(1, r, c, lvl);
                t->targetPos = t->currentPos;
                t->scale = 1.0f;
                t->isAnimating = false;
                t->type = getRandomShapeType();
            }
        }
    }

    // 3. Base Layer 0 (8x8 wall, 3 rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L0_DIM; ++r) {
            for (int c = 0; c < L0_DIM; ++c) {
                if (!isValidCoord({0, r, c, lvl})) continue;
                ShapeTile* t = getTile({0, r, c, lvl});
                t->layer = 0; t->row = r; t->col = c; t->level = lvl;
                t->currentPos = getSlotWorldPos(0, r, c, lvl);
                t->targetPos = t->currentPos;
                t->scale = 1.0f;
                t->isAnimating = false;
                t->type = getRandomShapeType();
            }
        }
    }

    // Re-roll accidental initial matches
    for (int attempts = 0; attempts < 30; ++attempts) {
        if (!checkAndMarkMatches()) break;
        for (const auto& coord : matchedCoords) {
            getTile(coord)->type = getRandomShapeType();
        }
        matchedCoords.clear();
    }
}

BoardCoord Board::pickTile(const Ray& ray) const {
    BoardCoord bestCoord;
    float closestDist = 1e9f;

    // Test Layer 2 (Top 4x4)
    for (int r = 0; r < L2_DIM; ++r) {
        for (int c = 0; c < L2_DIM; ++c) {
            BoardCoord coord{2, r, c, 0};
            const ShapeTile* t = getTile(coord);
            if (!t || t->type == ShapeType::NONE) continue;

            glm::vec3 oc = ray.origin - t->currentPos;
            float b = glm::dot(oc, ray.direction);
            float cVal = glm::dot(oc, oc) - (0.50f * 0.50f);
            float disc = b * b - cVal;
            if (disc >= 0.0f) {
                float tHit = -b - std::sqrt(disc);
                if (tHit > 0.1f && tHit < closestDist) {
                    closestDist = tHit;
                    bestCoord = coord;
                }
            }
        }
    }

    // Test Layer 1 (6x6 wall, 3 rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L1_DIM; ++r) {
            for (int c = 0; c < L1_DIM; ++c) {
                BoardCoord coord{1, r, c, lvl};
                if (!isValidCoord(coord)) continue;
                const ShapeTile* t = getTile(coord);
                if (!t || t->type == ShapeType::NONE) continue;

                glm::vec3 oc = ray.origin - t->currentPos;
                float b = glm::dot(oc, ray.direction);
                float cVal = glm::dot(oc, oc) - (0.50f * 0.50f);
                float disc = b * b - cVal;
                if (disc >= 0.0f) {
                    float tHit = -b - std::sqrt(disc);
                    if (tHit > 0.1f && tHit < closestDist) {
                        closestDist = tHit;
                        bestCoord = coord;
                    }
                }
            }
        }
    }

    // Test Layer 0 (8x8 wall, 3 rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L0_DIM; ++r) {
            for (int c = 0; c < L0_DIM; ++c) {
                BoardCoord coord{0, r, c, lvl};
                if (!isValidCoord(coord)) continue;
                const ShapeTile* t = getTile(coord);
                if (!t || t->type == ShapeType::NONE) continue;

                glm::vec3 oc = ray.origin - t->currentPos;
                float b = glm::dot(oc, ray.direction);
                float cVal = glm::dot(oc, oc) - (0.50f * 0.50f);
                float disc = b * b - cVal;
                if (disc >= 0.0f) {
                    float tHit = -b - std::sqrt(disc);
                    if (tHit > 0.1f && tHit < closestDist) {
                        closestDist = tHit;
                        bestCoord = coord;
                    }
                }
            }
        }
    }

    return bestCoord;
}

void Board::onTileClicked(const BoardCoord& clicked) {
    if (state != GameState::IDLE) return;
    if (!isValidCoord(clicked)) return;

    if (!selectedCoord.isValid()) {
        selectedCoord = clicked;
        const ShapeTile* t = getTile(clicked);
        std::cout << "[Selected] Layer " << clicked.layer << " (" << clicked.row << ", " << clicked.col
                  << ", level " << clicked.level << ") - " << getShapeName(t ? t->type : ShapeType::NONE) << std::endl;
        if (t) {
            std::cout << "  [Transform: Model] " << getShapeName(t->type) << "\n"
                      << "    Translation : T( " << t->currentPos.x << ", " << t->currentPos.y << ", " << t->currentPos.z << " )\n"
                      << "    Rotation    : R_y( active hover + continuous spin )\n"
                      << "    Scale       : S( " << t->scale << ", " << t->scale << ", " << t->scale << " )" << std::endl;
        }
        return;
    }

    if (selectedCoord == clicked) {
        selectedCoord = BoardCoord(); // Deselect
        std::cout << "[Deselected]" << std::endl;
        return;
    }

    if (isAdjacent(selectedCoord, clicked)) {
        swapTargetCoord = clicked;
        executeSwap(selectedCoord, swapTargetCoord, false);
    } else {
        selectedCoord = clicked;
        const ShapeTile* t = getTile(clicked);
        std::cout << "[Selected] Layer " << clicked.layer << " (" << clicked.row << ", " << clicked.col
                  << ", level " << clicked.level << ") - " << getShapeName(t ? t->type : ShapeType::NONE) << std::endl;
        if (t) {
            std::cout << "  [Transform: Model] " << getShapeName(t->type) << "\n"
                      << "    Translation : T( " << t->currentPos.x << ", " << t->currentPos.y << ", " << t->currentPos.z << " )\n"
                      << "    Rotation    : R_y( active hover + continuous spin )\n"
                      << "    Scale       : S( " << t->scale << ", " << t->scale << ", " << t->scale << " )" << std::endl;
        }
    }
}

void Board::executeSwap(const BoardCoord& a, const BoardCoord& b, bool isRevert) {
    ShapeTile* tA = getTile(a);
    ShapeTile* tB = getTile(b);
    if (!tA || !tB) return;

    glm::vec3 posA = getSlotWorldPos(a.layer, a.row, a.col, a.level);
    glm::vec3 posB = getSlotWorldPos(b.layer, b.row, b.col, b.level);

    if (!isRevert) {
        std::cout << "[Transform: Swap Arc] Swapping Layer " << a.layer << " (" << a.row << "," << a.col << ", Lvl " << a.level << ") <-> ("
                  << b.row << "," << b.col << ", Lvl " << b.level << ")\n"
                  << "  Tile A Pos : [ " << posA.x << ", " << posA.y << ", " << posA.z << " ] -> [ " << posB.x << ", " << posB.y << ", " << posB.z << " ]\n"
                  << "  Tile B Pos : [ " << posB.x << ", " << posB.y << ", " << posB.z << " ] -> [ " << posA.x << ", " << posA.y << ", " << posA.z << " ]\n"
                  << "  Duration   : 0.20 sec (SmoothStep interpolation)" << std::endl;
    }

    // 3D Arc-hop trajectories
    tA->startMove(posB, 0.20f, 0.40f);
    tB->startMove(posA, 0.20f, -0.20f);

    state = isRevert ? GameState::REVERTING : GameState::SWAPPING;
    stateTimer = 0.22f;
}

bool Board::checkAndMarkMatches() {
    matchedCoords.clear();

    // 1. Scan Top Layer 2 (4x4 Grid)
    for (int r = 0; r < L2_DIM; ++r) {
        for (int c = 0; c < L2_DIM - 2; ++c) {
            ShapeType t0 = getTile({2, r, c, 0})->type;
            ShapeType t1 = getTile({2, r, c + 1, 0})->type;
            ShapeType t2 = getTile({2, r, c + 2, 0})->type;
            if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                matchedCoords.insert({2, r, c, 0});
                matchedCoords.insert({2, r, c + 1, 0});
                matchedCoords.insert({2, r, c + 2, 0});
            }
        }
    }
    for (int c = 0; c < L2_DIM; ++c) {
        for (int r = 0; r < L2_DIM - 2; ++r) {
            ShapeType t0 = getTile({2, r, c, 0})->type;
            ShapeType t1 = getTile({2, r + 1, c, 0})->type;
            ShapeType t2 = getTile({2, r + 2, c, 0})->type;
            if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                matchedCoords.insert({2, r, c, 0});
                matchedCoords.insert({2, r + 1, c, 0});
                matchedCoords.insert({2, r + 2, c, 0});
            }
        }
    }

    // 2. Scan Vertical Walls (Layer 1 and Layer 0)
    auto scanWall = [&](int layer, int size) {
        // A. Horizontal Matches (around each ring level)
        for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
            std::vector<BoardCoord> ring;
            for (int c = 0; c < size; ++c) ring.push_back({layer, 0, c, lvl});
            for (int r = 1; r < size; ++r) ring.push_back({layer, r, size - 1, lvl});
            for (int c = size - 2; c >= 0; --c) ring.push_back({layer, size - 1, c, lvl});
            for (int r = size - 2; r >= 1; --r) ring.push_back({layer, r, 0, lvl});

            int N = static_cast<int>(ring.size());
            for (int i = 0; i < N; ++i) {
                ShapeType t0 = getTile(ring[i])->type;
                ShapeType t1 = getTile(ring[(i + 1) % N])->type;
                ShapeType t2 = getTile(ring[(i + 2) % N])->type;
                if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                    matchedCoords.insert(ring[i]);
                    matchedCoords.insert(ring[(i + 1) % N]);
                    matchedCoords.insert(ring[(i + 2) % N]);
                }
            }
        }

        // B. Vertical Matches (up and down each perimeter column)
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                if (!isValidCoord({layer, r, c, 0})) continue;
                ShapeType t0 = getTile({layer, r, c, 0})->type;
                ShapeType t1 = getTile({layer, r, c, 1})->type;
                ShapeType t2 = getTile({layer, r, c, 2})->type;
                if (t0 != ShapeType::NONE && t0 == t1 && t1 == t2) {
                    matchedCoords.insert({layer, r, c, 0});
                    matchedCoords.insert({layer, r, c, 1});
                    matchedCoords.insert({layer, r, c, 2});
                }
            }
        }
    };

    scanWall(1, L1_DIM); // 6x6 wall: 60 horizontal + 20 vertical match lines
    scanWall(0, L0_DIM); // 8x8 wall: 84 horizontal + 28 vertical match lines

    return !matchedCoords.empty();
}

void Board::applyGravityAndRefill() {
    // 1. Top Layer 2 Gravity Drop & Refill
    for (int c = 0; c < L2_DIM; ++c) {
        int writeRow = L2_DIM - 1;
        for (int r = L2_DIM - 1; r >= 0; --r) {
            ShapeTile* t = getTile({2, r, c, 0});
            if (t->type != ShapeType::NONE) {
                if (writeRow != r) {
                    ShapeTile* targetTile = getTile({2, writeRow, c, 0});
                    targetTile->type = t->type;
                    targetTile->scale = 1.0f;
                    targetTile->currentPos = t->currentPos;
                    targetTile->startMove(getSlotWorldPos(2, writeRow, c, 0), 0.22f);
                    t->type = ShapeType::NONE;
                }
                writeRow--;
            }
        }
        for (int r = writeRow; r >= 0; --r) {
            ShapeTile* t = getTile({2, r, c, 0});
            t->type = getRandomShapeType();
            t->scale = 1.0f;
            glm::vec3 spawnPos = getSlotWorldPos(2, r, c, 0) + glm::vec3(0.0f, 1.8f, 0.0f);
            t->currentPos = spawnPos;
            t->startMove(getSlotWorldPos(2, r, c, 0), 0.24f);
        }
    }

    // 2. Wall Layers 1 & 0 Vertical Gravity (falling down the wall towards level 0)
    auto applyWallGravity = [&](int layer, int size) {
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                if (!isValidCoord({layer, r, c, 0})) continue;

                int writeLevel = 0;
                for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
                    ShapeTile* t = getTile({layer, r, c, lvl});
                    if (t->type != ShapeType::NONE) {
                        if (writeLevel != lvl) {
                            ShapeTile* targetTile = getTile({layer, r, c, writeLevel});
                            targetTile->type = t->type;
                            targetTile->scale = 1.0f;
                            targetTile->currentPos = t->currentPos;
                            targetTile->startMove(getSlotWorldPos(layer, r, c, writeLevel), 0.22f);
                            t->type = ShapeType::NONE;
                        }
                        writeLevel++;
                    }
                }

                // Spawn new shapes at the top of the wall (level 2) and drop down
                for (int lvl = writeLevel; lvl < RINGS_PER_WALL; ++lvl) {
                    ShapeTile* t = getTile({layer, r, c, lvl});
                    t->type = getRandomShapeType();
                    t->scale = 1.0f;
                    glm::vec3 spawnPos = getSlotWorldPos(layer, r, c, RINGS_PER_WALL - 1) + glm::vec3(0.0f, 1.2f, 0.0f);
                    t->currentPos = spawnPos;
                    t->startMove(getSlotWorldPos(layer, r, c, lvl), 0.24f);
                }
            }
        }
    };

    applyWallGravity(1, L1_DIM);
    applyWallGravity(0, L0_DIM);
    std::cout << "  [Transform: Wall Gravity & Spawn] Shapes dropping down vertical wall facades" << std::endl;
}

void Board::update(float deltaTime) {
    // Smoothly animate Exploded View (E key)
    float targetExplode = isExploded ? 1.0f : 0.0f;
    if (std::abs(explodeProgress - targetExplode) > 0.0005f) {
        explodeProgress = glm::mix(explodeProgress, targetExplode, deltaTime * 5.0f);

        // Update positions of non-animating shapes
        for (int r = 0; r < L2_DIM; ++r) {
            for (int c = 0; c < L2_DIM; ++c) {
                ShapeTile* t = getTile({2, r, c, 0});
                if (t && !t->isAnimating) {
                    t->currentPos = getSlotWorldPos(2, r, c, 0);
                    t->targetPos = t->currentPos;
                }
            }
        }

        for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
            for (int r = 0; r < L1_DIM; ++r) {
                for (int c = 0; c < L1_DIM; ++c) {
                    if (isValidCoord({1, r, c, lvl})) {
                        ShapeTile* t = getTile({1, r, c, lvl});
                        if (t && !t->isAnimating) {
                            t->currentPos = getSlotWorldPos(1, r, c, lvl);
                            t->targetPos = t->currentPos;
                        }
                    }
                }
            }
        }
    }

    // Update animations for all shapes
    for (int r = 0; r < L2_DIM; ++r)
        for (int c = 0; c < L2_DIM; ++c)
            getTile({2, r, c, 0})->update(deltaTime);

    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl)
        for (int r = 0; r < L1_DIM; ++r)
            for (int c = 0; c < L1_DIM; ++c)
                if (isValidCoord({1, r, c, lvl})) getTile({1, r, c, lvl})->update(deltaTime);

    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl)
        for (int r = 0; r < L0_DIM; ++r)
            for (int c = 0; c < L0_DIM; ++c)
                if (isValidCoord({0, r, c, lvl})) getTile({0, r, c, lvl})->update(deltaTime);

    if (stateTimer > 0.0f) {
        stateTimer -= deltaTime;
    }

    switch (state) {
        case GameState::SWAPPING: {
            if (stateTimer <= 0.0f) {
                ShapeTile* tA = getTile(selectedCoord);
                ShapeTile* tB = getTile(swapTargetCoord);
                if (tA && tB) {
                    std::swap(tA->type, tB->type);
                    tA->currentPos = getSlotWorldPos(selectedCoord.layer, selectedCoord.row, selectedCoord.col, selectedCoord.level);
                    tB->currentPos = getSlotWorldPos(swapTargetCoord.layer, swapTargetCoord.row, swapTargetCoord.col, swapTargetCoord.level);
                }

                if (checkAndMarkMatches()) {
                    combo = 1;
                    int points = static_cast<int>(matchedCoords.size()) * 100 * combo;
                    score += points;
                    std::cout << "[MATCH] " << matchedCoords.size() << " shapes crushed! +" << points << " pts! (Combo " << combo << "x)" << std::endl;
                    std::cout << "  [Transform: Pop] Crushing matched shapes: Scale S( 1.000 -> 0.000 )" << std::endl;

                    for (const auto& c : matchedCoords) {
                        getTile(c)->startPop(0.22f);
                    }
                    state = GameState::POPPING;
                    stateTimer = 0.24f;
                } else {
                    std::cout << "[NO MATCH] Shapes did not line up. Reverting..." << std::endl;
                    executeSwap(selectedCoord, swapTargetCoord, true);
                }
            }
            break;
        }
        case GameState::REVERTING: {
            if (stateTimer <= 0.0f) {
                ShapeTile* tA = getTile(selectedCoord);
                ShapeTile* tB = getTile(swapTargetCoord);
                if (tA && tB) {
                    std::swap(tA->type, tB->type);
                    tA->currentPos = getSlotWorldPos(selectedCoord.layer, selectedCoord.row, selectedCoord.col, selectedCoord.level);
                    tB->currentPos = getSlotWorldPos(swapTargetCoord.layer, swapTargetCoord.row, swapTargetCoord.col, swapTargetCoord.level);
                }
                selectedCoord = BoardCoord();
                swapTargetCoord = BoardCoord();
                state = GameState::IDLE;
            }
            break;
        }
        case GameState::POPPING: {
            if (stateTimer <= 0.0f) {
                applyGravityAndRefill();
                state = GameState::FALLING;
                stateTimer = 0.26f;
            }
            break;
        }
        case GameState::FALLING: {
            if (stateTimer <= 0.0f) {
                if (checkAndMarkMatches()) {
                    combo++;
                    int points = static_cast<int>(matchedCoords.size()) * 150 * combo;
                    score += points;
                    std::cout << "[CASCADE COMBO " << combo << "x] +" << points << " pts!" << std::endl;
                    std::cout << "  [Transform: Pop] Crushing matched shapes: Scale S( 1.000 -> 0.000 )" << std::endl;

                    for (const auto& c : matchedCoords) {
                        getTile(c)->startPop(0.22f);
                    }
                    state = GameState::POPPING;
                    stateTimer = 0.24f;
                } else {
                    combo = 0;
                    selectedCoord = BoardCoord();
                    swapTargetCoord = BoardCoord();
                    state = GameState::IDLE;
                }
            }
            break;
        }
        default:
            break;
    }
}

void Board::draw(const Shader& shader, const Mesh& cubeMesh, const Mesh& sphereMesh,
                 const Mesh& cylinderMesh, const Mesh& diamondMesh, float time) const {
    // -------------------------------------------------------------
    // 1. DRAW ARCHITECTURAL STEPPED TRAYS & CORE PEDESTALS
    // -------------------------------------------------------------
    shader.setBool("isTray", true);
    shader.setVec4("objectColor", glm::vec4(0.38f, 0.42f, 0.52f, 1.0f));

    float explode1 = 1.80f * explodeProgress;
    float explode2 = 3.60f * explodeProgress;

    // Layer 0 Base Tray (Foundation: 8.5 x 0.25 x 8.5) at Y = 0.0
    glm::mat4 m0 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    m0 = glm::scale(m0, glm::vec3(8.5f, 0.25f, 8.5f));
    shader.setMat4("model", m0);
    cubeMesh.draw();

    // Layer 0 Core Pedestal (Solid stone core inside the 8x8 wall)
    glm::mat4 c0 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.425f, 0.0f));
    c0 = glm::scale(c0, glm::vec3(6.0f, 2.60f, 6.0f));
    shader.setMat4("model", c0);
    cubeMesh.draw();

    // Layer 1 Middle Tray (Terrace Platform: 6.5 x 0.25 x 6.5) at Y = 2.85 + explode1
    glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.85f + explode1, 0.0f));
    m1 = glm::scale(m1, glm::vec3(6.5f, 0.25f, 6.5f));
    shader.setMat4("model", m1);
    cubeMesh.draw();

    // Layer 1 Core Pedestal (Solid stone core inside the 6x6 wall)
    glm::mat4 c1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.275f + explode1, 0.0f));
    c1 = glm::scale(c1, glm::vec3(4.0f, 2.60f, 4.0f));
    shader.setMat4("model", c1);
    cubeMesh.draw();

    // Layer 2 Top Tray (Summit Platform: 4.5 x 0.25 x 4.5) at Y = 5.70 + explode2
    glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.70f + explode2, 0.0f));
    m2 = glm::scale(m2, glm::vec3(4.5f, 0.25f, 4.5f));
    shader.setMat4("model", m2);
    cubeMesh.draw();

    // -------------------------------------------------------------
    // 2. DRAW SHAPES (Top Board + 3-Story Colosseum Walls)
    // -------------------------------------------------------------
    shader.setBool("isTray", false);

    // Draw Top Layer 2 (4x4)
    for (int r = 0; r < L2_DIM; ++r) {
        for (int c = 0; c < L2_DIM; ++c) {
            BoardCoord coord{2, r, c, 0};
            bool isSel = (coord == selectedCoord);
            getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
        }
    }

    // Draw Middle Layer 1 (6x6 wall, 3 vertical rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L1_DIM; ++r) {
            for (int c = 0; c < L1_DIM; ++c) {
                BoardCoord coord{1, r, c, lvl};
                if (!isValidCoord(coord)) continue;
                bool isSel = (coord == selectedCoord);
                getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
            }
        }
    }

    // Draw Base Layer 0 (8x8 wall, 3 vertical rings)
    for (int lvl = 0; lvl < RINGS_PER_WALL; ++lvl) {
        for (int r = 0; r < L0_DIM; ++r) {
            for (int c = 0; c < L0_DIM; ++c) {
                BoardCoord coord{0, r, c, lvl};
                if (!isValidCoord(coord)) continue;
                bool isSel = (coord == selectedCoord);
                getTile(coord)->draw(shader, sphereMesh, cylinderMesh, cubeMesh, diamondMesh, isSel, time);
            }
        }
    }
}
