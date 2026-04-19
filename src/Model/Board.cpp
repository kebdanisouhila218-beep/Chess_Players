#include "Board.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
    struct SextantInterval {
        /// Structure représentant un intervalle de coordonnées x et y pour un sextant   
        int x0; //q
        int x1;
        int y0; //r
        int y1;
    };
//tableau des sextants
    constexpr std::array<SextantInterval, 6> kIntervals = {
        SextantInterval{0, 4, 0, 4},  //JB cote gauche 
        SextantInterval{0, 4, 4, 8},  //JB cote bas  
        SextantInterval{8, 12, 4, 8},
        SextantInterval{8, 12, 8, 12},
        SextantInterval{4, 8, 8, 12},
        SextantInterval{4, 8, 0, 4}   //JB cote droit
    };
}

Board::Board() {
    for (auto& row : xyToId) {
        row.fill(-1);
    }
    for (auto& row : neighbors) {
        row.fill(-1);
    }

    int id = 0;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const int sextant = sextantFor(x, y);
            if (sextant < 0) {
                continue;
            }

            BoardNode node;
            node.id = id;
            node.cell = {x, y};
            node.sextant = sextant;
            node.zoneOwner = zoneForSextant(sextant);
            node.screen = toScreen(x, y);

            nodes[id] = node;
            xyToId[y][x] = id;
            cells[node.cell] = nullptr;
            ++id;
        }
    }

    buildNeighbors();
}

int Board::sextantFor(int x, int y) {
    for (int i = 0; i < static_cast<int>(kIntervals.size()); ++i) {
        const SextantInterval& interval = kIntervals[i];
        if (x >= interval.x0 && x < interval.x1 && y >= interval.y0 && y < interval.y1) {
            return i;
        }
    }
    return -1;
}

ScreenCoord Board::toScreen(int x, int y) {
    const float cx = 490.f;
    const float cy = 360.f;
    const float colStep = 34.f;
    const float rowStep = 28.f;
    const float shear = 10.f;

    const float dx = static_cast<float>(x) - 5.5f;
    const float dy = static_cast<float>(y) - 5.5f;
    return {cx + dx * colStep + dy * shear, cy + dy * rowStep};
}

Player Board::zoneForSextant(int sextant) {
    if (sextant == 0 || sextant == 1) return Player::PLAYER1;
    if (sextant == 2 || sextant == 3) return Player::PLAYER2;
    return Player::PLAYER3;
}

void Board::linkNeighbor(const HexCell& from, Direction dir, const HexCell& to) {
    const int fromId = getId(from);
    const int toId = getId(to);
    if (fromId < 0 || toId < 0) {
        return;
    }
    neighbors[fromId][static_cast<int>(dir)] = toId;
}

void Board::buildNeighbors() {
    //Graphe de voisinage
    for (const BoardNode& node : nodes) {
        const int x = node.cell.q;
        const int y = node.cell.r;

        const std::array<std::pair<Direction, HexCell>, 8> localDirs = {{
            {Direction::NORTH, {x, y - 1}},
            {Direction::SOUTH, {x, y + 1}},
            {Direction::EAST, {x + 1, y}},
            {Direction::WEST, {x - 1, y}},
            {Direction::NORTH_EAST, {x + 1, y - 1}},
            {Direction::SOUTH_WEST, {x - 1, y + 1}},
            {Direction::SOUTH_EAST, {x + 1, y + 1}},
            {Direction::NORTH_WEST, {x - 1, y - 1}}
        }};

        for (const auto& [dir, target] : localDirs) {
            if (isValid(target)) {
                linkNeighbor(node.cell, dir, target);
            }
        }
    }

    // Couture 1 : bord est du sextant haut droit (x=7, y=0..3)
    // vers bord nord du sextant bas central (x=4..7, y=11).
    // Cette liaison relie la zone PLAYER1 a la zone PLAYER3.
    for (int k = 0; k < 4; ++k) {
        linkNeighbor({7, k}, Direction::SOUTH, {4 + k, 11});
        linkNeighbor({4 + k, 11}, Direction::NORTH, {7, k});

        if (k > 0) {
                    //ا تقم بعملية حسابية، بل انقلها فجأة (Teleport
            linkNeighbor({7, k}, Direction::SOUTH_WEST, {3 + k, 11});
            linkNeighbor({3 + k, 11}, Direction::NORTH_EAST, {7, k});
        }
        if (k < 3) {
            linkNeighbor({7, k}, Direction::SOUTH_EAST, {5 + k, 11});
            linkNeighbor({5 + k, 11}, Direction::NORTH_WEST, {7, k});
        }

        // Couture 2 : bord sud du sextant haut gauche (x=0..3, y=3)
        // vers bord ouest du sextant central gauche (x=3, y=4..7).
        // Cette liaison reste dans la zone PLAYER1, mais change de sextant.
        linkNeighbor({k, 3}, Direction::SOUTH, {3, 4 + k});
        linkNeighbor({3, 4 + k}, Direction::NORTH, {k, 3});

        if (k > 0) {
            linkNeighbor({k, 3}, Direction::SOUTH_WEST, {3, 3 + k});
            linkNeighbor({3, 3 + k}, Direction::NORTH_EAST, {k, 3});
        }
        if (k < 3) {
            linkNeighbor({k, 3}, Direction::SOUTH_EAST, {3, 5 + k});
            linkNeighbor({3, 5 + k}, Direction::NORTH_WEST, {k, 3});
        }

        // Couture 3 : bord sud du sextant central droit (x=8..11, y=7)
        // vers bord est du sextant bas droit (x=11, y=8..11).
        // Cette liaison reste dans la zone PLAYER2, mais change de sextant.
        linkNeighbor({8 + k, 7}, Direction::SOUTH, {11, 8 + k});
        linkNeighbor({11, 8 + k}, Direction::NORTH, {8 + k, 7});

        if (k > 0) {
            linkNeighbor({8 + k, 7}, Direction::SOUTH_WEST, {11, 7 + k});
            linkNeighbor({11, 7 + k}, Direction::NORTH_EAST, {8 + k, 7});
        }
        if (k < 3) {
            linkNeighbor({8 + k, 7}, Direction::SOUTH_EAST, {11, 9 + k});
            linkNeighbor({11, 9 + k}, Direction::NORTH_WEST, {8 + k, 7});
        }
    }
}

bool Board::isValid(const HexCell& c) const {
    return getId(c) >= 0;
}

Piece* Board::getPiece(const HexCell& c) const {
    auto it = cells.find(c);
    if (it != cells.end()) {
        return it->second;
    }
    return nullptr;
}

void Board::setPiece(const HexCell& c, Piece* p) {
    if (isValid(c)) {
        cells[c] = p;
    }
}

void Board::removePiece(const HexCell& c) {
    if (isValid(c)) {
        cells[c] = nullptr;
    }
}

void Board::movePiece(const HexCell& from, const HexCell& to) {
    Piece* p = getPiece(from);
    if (!p) {
        return;
    }

    setPiece(to, p);
    removePiece(from);
    p->setPos(to);
}

std::vector<HexCell> Board::allValidCells() const {
    std::vector<HexCell> result;
    result.reserve(CELL_COUNT);
    for (const BoardNode& node : nodes) {
        result.push_back(node.cell);
    }
    return result;
}

int Board::getId(const HexCell& c) const {
    if (c.q < 0 || c.q >= BOARD_SIZE || c.r < 0 || c.r >= BOARD_SIZE) {
        return -1;
    }
    return xyToId[c.r][c.q];
}

HexCell Board::getCellById(int id) const {
    if (id < 0 || id >= CELL_COUNT) {
        return {-1, -1};
    }
    return nodes[id].cell;
}

const BoardNode* Board::getNode(const HexCell& c) const {
    const int id = getId(c);
    if (id < 0) {
        return nullptr;
    }
    return &nodes[id];
}

const BoardNode* Board::getNodeById(int id) const {
    if (id < 0 || id >= CELL_COUNT) {
        return nullptr;
    }
    return &nodes[id];
}

ScreenCoord Board::getScreenPosition(const HexCell& c) const {
    const BoardNode* node = getNode(c);
    if (!node) {
        return {0.f, 0.f};
    }
    return node->screen;
}

std::optional<HexCell> Board::pickCellFromScreen(float px, float py, float maxDistance) const {
    const BoardNode* bestNode = nullptr;
    float bestDist2 = maxDistance * maxDistance;

    for (const BoardNode& node : nodes) {
        const float dx = node.screen.x - px;
        const float dy = node.screen.y - py;
        const float dist2 = dx * dx + dy * dy;
        if (!bestNode || dist2 < bestDist2) {
            if (dist2 <= maxDistance * maxDistance) {
                bestNode = &node;
                bestDist2 = dist2;
            }
        }
    }

    if (!bestNode) {
        return std::nullopt;
    }
    return bestNode->cell;
}

int Board::getSextant(const HexCell& c) const {
    const BoardNode* node = getNode(c);
    return node ? node->sextant : -1;
}

Player Board::getZoneOwner(const HexCell& c) const {
    const BoardNode* node = getNode(c);
    return node ? node->zoneOwner : Player::NONE;
}

std::optional<HexCell> Board::step(const HexCell& from, Direction dir) const {
    const int fromId = getId(from);
    if (fromId < 0) {
        return std::nullopt;
    }
    const int toId = neighbors[fromId][static_cast<int>(dir)];
    if (toId < 0) {
        return std::nullopt;
    }
    return getCellById(toId);
}

std::vector<HexCell> Board::ray(const HexCell& from, Direction dir) const {
    std::vector<HexCell> result;
    std::optional<HexCell> current = step(from, dir);
    while (current.has_value()) {
        result.push_back(*current);
        current = step(*current, dir);
    }
    return result;
}

std::optional<HexCell> Board::getPawnTransition(const HexCell& from, Player owner) const {
    const int x = from.q;
    const int y = from.r;

    if (owner == Player::PLAYER1) {
        if (x == 7 && y >= 0 && y <= 3) return HexCell{4 + y, 11};
        if (y == 11 && x >= 4 && x <= 7) return HexCell{x, 10};
        if (y == 10 && x >= 4 && x <= 7) return HexCell{x, 9};
        if (y == 3 && x >= 0 && x <= 3) return HexCell{3, 4 + x};
        if (x == 3 && y >= 4 && y <= 7) return HexCell{2, y};
        if (x == 2 && y >= 4 && y <= 7) return HexCell{1, y};
        return std::nullopt;
    }

    if (owner == Player::PLAYER2) {
        if (x == 3 && y >= 4 && y <= 7) return HexCell{y - 4, 3};
        if (y == 3 && x >= 0 && x <= 3) return HexCell{x, 2};
        if (y == 7 && x >= 8 && x <= 11) return HexCell{11, x};
        if (x == 11 && y >= 8 && y <= 11) return HexCell{10, y};
        return std::nullopt;
    }

    if (owner == Player::PLAYER3) {
        if (x == 11 && y >= 8 && y <= 11) return HexCell{y, 7};
        if (y == 7 && x >= 8 && x <= 11) return HexCell{x, 6};
        if (y == 11 && x >= 4 && x <= 7) return HexCell{7, x - 4};
        if (x == 7 && y >= 0 && y <= 3) return HexCell{6, y};
    }
    return std::nullopt;
}

bool Board::isPromotionCell(const HexCell& c, Player owner) const {
    if (!isValid(c)) {
        return false;
    }

    const auto isOneOf = [&](std::initializer_list<HexCell> cells) {
        for (const HexCell& cell : cells) {
            if (c == cell) {
                return true;
            }
        }
        return false;
    };

    if (owner == Player::PLAYER1) {
        return isOneOf({
            {0, 4}, {0, 5}, {0, 6}, {0, 7},
            {8, 4}, {9, 4}, {10, 4}, {11, 4},
            {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8}
        });
    }
    if (owner == Player::PLAYER2) {
        return isOneOf({
            {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
            {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8}
        });
    }
    return isOneOf({
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
        {0, 4}, {0, 5}, {0, 6}, {0, 7},
        {8, 4}, {9, 4}, {10, 4}, {11, 4}
    });
}