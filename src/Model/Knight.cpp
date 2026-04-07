#include "Knight.hpp"
#include "Board.hpp"

#include <algorithm>
#include <array>

std::vector<HexCell> Knight::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    const std::array<std::array<Board::Direction, 3>, 8> patterns = {{
        {Board::Direction::NORTH, Board::Direction::NORTH, Board::Direction::EAST},
        {Board::Direction::NORTH, Board::Direction::NORTH, Board::Direction::WEST},
        {Board::Direction::SOUTH, Board::Direction::SOUTH, Board::Direction::EAST},
        {Board::Direction::SOUTH, Board::Direction::SOUTH, Board::Direction::WEST},
        {Board::Direction::EAST, Board::Direction::EAST, Board::Direction::NORTH},
        {Board::Direction::EAST, Board::Direction::EAST, Board::Direction::SOUTH},
        {Board::Direction::WEST, Board::Direction::WEST, Board::Direction::NORTH},
        {Board::Direction::WEST, Board::Direction::WEST, Board::Direction::SOUTH}
    }};

    for (const auto& pattern : patterns) {
        std::optional<HexCell> current = pos;
        for (Board::Direction dir : pattern) {
            current = current.has_value() ? board.step(*current, dir) : std::nullopt;
            if (!current.has_value()) {
                break;
            }
        }

        if (!current.has_value()) {
            continue;
        }

        Piece* target = board.getPiece(*current);
        if (target == nullptr || target->getOwner() != owner) {
            moves.push_back(*current);
        }
    }

    std::sort(moves.begin(), moves.end(), [](const HexCell& a, const HexCell& b) {
        return a.q < b.q || (a.q == b.q && a.r < b.r);
    });
    moves.erase(std::unique(moves.begin(), moves.end()), moves.end());

    return moves;
}