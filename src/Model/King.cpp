#include "King.hpp"
#include "Board.hpp"

std::vector<HexCell> King::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    // Le roi se déplace d'une case dans les 6 directions
    std::vector<HexCell> directions = {
        {+1,  0}, {-1,  0},
        { 0, +1}, { 0, -1},
        {+1, -1}, {-1, +1}
    };

    for (const HexCell& dir : directions) {
        HexCell target = {pos.q + dir.q, pos.r + dir.r};
        if (!board.isValid(target)) continue;
        Piece* p = board.getPiece(target);
        if (p == nullptr || p->getOwner() != owner)
            moves.push_back(target);
    }

    return moves;
}