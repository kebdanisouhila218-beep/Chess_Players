#pragma once
#include "Piece.hpp"

class Pawn : public Piece {
public:
    Pawn(Player owner, HexCell pos)
        : Piece(PieceType::PAWN, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 1; }
};