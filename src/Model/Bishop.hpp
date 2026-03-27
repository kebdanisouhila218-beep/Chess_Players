#pragma once
#include "Piece.hpp"

class Bishop : public Piece {
public:
    Bishop(Player owner, HexCell pos)
        : Piece(PieceType::BISHOP, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 3; }
};