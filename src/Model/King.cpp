#include "King.hpp"
#include "Board.hpp"

// Le roi se deplace d'une case dans les 8 directions via Board::step().
// Il ne peut pas capturer ses propres pieces.
// Le roque n'est pas implemente : la geometrie du plateau 3 joueurs
// (roi et tours sur des rangees differentes) ne permet pas le roque classique.
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