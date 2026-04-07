#include "King.hpp"
#include "Board.hpp"

std::vector<HexCell> King::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    const std::array<Board::Direction, 8> directions = {
        Board::Direction::NORTH,
        Board::Direction::SOUTH,
        Board::Direction::EAST,
        Board::Direction::WEST,
        Board::Direction::NORTH_EAST,
        Board::Direction::SOUTH_WEST,
        Board::Direction::SOUTH_EAST,
        Board::Direction::NORTH_WEST
    };

    for (Board::Direction dir : directions) {
        std::optional<HexCell> target = board.step(pos, dir);
        if (!target.has_value()) continue;

        Piece* p = board.getPiece(*target);
        if (p == nullptr || p->getOwner() != owner) {
            moves.push_back(*target);
        }
    }

    return moves;
}