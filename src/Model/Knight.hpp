#pragma once
#include "Piece.hpp"

class Knight : public Piece {
public:
    Knight(Player owner, HexCell pos)
        : Piece(PieceType::KNIGHT, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 3; }
};