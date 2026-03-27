#pragma once
#include "Board.hpp"
#include "Pawn.hpp"
#include "Knight.hpp"
#include "Bishop.hpp"
#include "Rook.hpp"
#include "Queen.hpp"
#include "King.hpp"

class PieceFactory {
public:
    Piece* create(PieceType type, Player owner, HexCell pos);
    void   initBoard(Board& board, Player player);
};