#pragma once
#include "HexCell.hpp"
#include "Piece.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

class Board {
public:
    Board();

    bool   isValid(const HexCell& c) const;
    Piece* getPiece(const HexCell& c) const;
    void   setPiece(const HexCell& c, Piece* p);
    void   removePiece(const HexCell& c);
    void   movePiece(const HexCell& from, const HexCell& to);

    std::vector<HexCell> allValidCells() const;

    static const int RADIUS = 5;

private:
    std::unordered_map<HexCell, Piece*, HexHash> cells;
};