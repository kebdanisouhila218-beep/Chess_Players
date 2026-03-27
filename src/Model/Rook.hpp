#pragma once
#include "Piece.hpp"

class Rook : public Piece {
public:
    Rook(Player owner, HexCell pos)
        : Piece(PieceType::ROOK, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 5; }
};