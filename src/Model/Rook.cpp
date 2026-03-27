#include "Rook.hpp"
#include "Board.hpp"

std::vector<HexCell> Rook::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    // 6 directions droites de la tour sur plateau hexagonal
    std::vector<HexCell> directions = {
        {+1,  0}, {-1,  0},
        { 0, +1}, { 0, -1},
        {+1, -1}, {-1, +1}
    };

    for (const HexCell& dir : directions) {
        HexCell current = {pos.q + dir.q, pos.r + dir.r};
        while (board.isValid(current)) {
            Piece* target = board.getPiece(current);
            if (target == nullptr) {
                moves.push_back(current);
            } else {
                if (target->getOwner() != owner)
                    moves.push_back(current);
                break; // bloqué
            }
            current = {current.q + dir.q, current.r + dir.r};
        }
    }

    return moves;
}