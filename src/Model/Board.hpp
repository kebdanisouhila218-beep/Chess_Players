#pragma once
#include "HexCell.hpp"
#include "Piece.hpp"
#include <array>
#include <optional>
#include <vector>
#include <unordered_map>

struct ScreenCoord {
    float x;
    float y;
};

struct BoardNode {
    int id = -1;
    HexCell cell{0, 0};
    int sextant = -1;
    Player zoneOwner = Player::NONE;
    ScreenCoord screen{0.f, 0.f};
};

class Board {
public:
    enum class Direction {
        NORTH = 0,
        SOUTH,
        EAST,
        WEST,
        NORTH_EAST,
        SOUTH_WEST,
        SOUTH_EAST,
        NORTH_WEST
    };

    Board();

    bool   isValid(const HexCell& c) const;
    Piece* getPiece(const HexCell& c) const;
    void   setPiece(const HexCell& c, Piece* p);
    void   removePiece(const HexCell& c);
    void   movePiece(const HexCell& from, const HexCell& to);

    std::vector<HexCell> allValidCells() const;

    int getId(const HexCell& c) const;
    HexCell getCellById(int id) const;
    const BoardNode* getNode(const HexCell& c) const;
    const BoardNode* getNodeById(int id) const;
    ScreenCoord getScreenPosition(const HexCell& c) const;
    std::optional<HexCell> pickCellFromScreen(float px, float py, float maxDistance = 22.f) const;
    int getSextant(const HexCell& c) const;
    Player getZoneOwner(const HexCell& c) const;
    std::optional<HexCell> step(const HexCell& from, Direction dir) const;
    std::vector<HexCell> ray(const HexCell& from, Direction dir) const;

    std::optional<HexCell> getPawnTransition(const HexCell& from, Player owner) const;
    bool isPromotionCell(const HexCell& c, Player owner) const;

    static constexpr int BOARD_SIZE = 12;
    static constexpr int CELL_COUNT = 96;

private:
    static int sextantFor(int x, int y);
    static ScreenCoord toScreen(int x, int y);
    static Player zoneForSextant(int sextant);
    void buildNeighbors();
    void linkNeighbor(const HexCell& from, Direction dir, const HexCell& to);

    std::array<BoardNode, CELL_COUNT> nodes;
    std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> xyToId;
    std::array<std::array<int, 8>, CELL_COUNT> neighbors;
    std::unordered_map<HexCell, Piece*, HexHash> cells;
};