#pragma once
#include "Piece.hpp"

struct Move;

class Pawn : public Piece {
public:
    Pawn(Player owner, HexCell pos)
        : Piece(PieceType::PAWN, owner, pos) {}

    std::vector<HexCell> getMoves(const Board& board) const override;
    std::vector<HexCell> getMoves(const Board& board, const Move* lastMove) const;
    int getValue() const override { return 1; }
};