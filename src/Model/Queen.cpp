#include "Queen.hpp"
#include "Board.hpp"
#include <algorithm>
#include <unordered_set>

std::vector<HexCell> Queen::getMoves(const Board& board) const {
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

    std::sort(moves.begin(), moves.end(), [](const HexCell& a, const HexCell& b){
        return a.q < b.q || (a.q == b.q && a.r < b.r);
    });
    moves.erase(std::unique(moves.begin(), moves.end()), moves.end());
    
    return moves;
}