#pragma once
#include "Piece.hpp"

class Queen : public Piece {
public:
    Queen(Player owner, HexCell pos)
        : Piece(PieceType::QUEEN, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    int getValue() const override { return 9; }
};