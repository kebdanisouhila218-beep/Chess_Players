#include "Rook.hpp"
#include "Board.hpp"

std::vector<HexCell> Rook::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    const std::array<Board::Direction, 4> directions = {
        Board::Direction::NORTH,
        Board::Direction::SOUTH,
        Board::Direction::EAST,
        Board::Direction::WEST
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