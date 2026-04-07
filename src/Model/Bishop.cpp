#include "Bishop.hpp"
#include "Board.hpp"

std::vector<HexCell> Bishop::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    const std::array<Board::Direction, 4> directions = {
        Board::Direction::NORTH_EAST,
        Board::Direction::SOUTH_WEST,
        Board::Direction::SOUTH_EAST,
        Board::Direction::NORTH_WEST
    };

    for (Board::Direction dir : directions) {
        for (const HexCell& current : board.ray(pos, dir)) {
            Piece* target = board.getPiece(current);
            if (target == nullptr) {
                moves.push_back(current);
            } else {
                if (target->getOwner() != owner) {
                    moves.push_back(current);
                }
                break;
            }
        }
    }

    return moves;
}