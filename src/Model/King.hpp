#pragma once
#include "Piece.hpp"

class King : public Piece {
public:
    King(Player owner, HexCell pos)
        : Piece(PieceType::KING, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 100; }
};