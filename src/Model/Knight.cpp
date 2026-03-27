#include "Knight.hpp"
#include "Board.hpp"

std::vector<HexCell> Knight::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    // Les 12 sauts du cavalier sur plateau hexagonal
    std::vector<HexCell> jumps = {
        {pos.q+2, pos.r-1}, {pos.q+2, pos.r+1},
        {pos.q-2, pos.r+1}, {pos.q-2, pos.r-1},
        {pos.q+1, pos.r+2}, {pos.q-1, pos.r+2},
        {pos.q+1, pos.r-2}, {pos.q-1, pos.r-2},
        {pos.q+3, pos.r-1}, {pos.q-3, pos.r+1},
        {pos.q+1, pos.r+3}, {pos.q-1, pos.r-3}
    };

    for (const HexCell& c : jumps) {
        if (!board.isValid(c)) continue;
        Piece* target = board.getPiece(c);
        if (target == nullptr || target->getOwner() != owner)
            moves.push_back(c);
    }

    return moves;
}