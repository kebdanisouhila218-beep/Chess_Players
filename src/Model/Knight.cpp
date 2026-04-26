#include "Knight.hpp"
#include "Board.hpp"

#include <array>
#include <unordered_set>
#include <iostream>
#include <string>

namespace {
    // Correct the Knight step at sextant rotated geometry boundaries
    // board.step() returns arithmetically correct neighbors, but Knight geometry differs by sextant
    std::optional<HexCell> knightActualStep(const Board& board, const HexCell& pos, Board::Direction dir) {
        const std::optional<HexCell> cell = board.step(pos, dir);
        if (!cell.has_value()) return std::nullopt;

        const int sextant = board.getSextant(pos);

        // Sextant 5 (PLAYER1 right): geometry is rotated
        // SOUTH arithmetic = (x, y+1), but geometric SOUTH = (x-1, y)
        // WEST arithmetic = (x-1, y), but geometric WEST = (x, y+1)
        if (sextant == 5) {
            HexCell direct = pos;
            switch (dir) {
                case Board::Direction::SOUTH: direct = {pos.q - 1, pos.r}; break;
                case Board::Direction::WEST:  direct = {pos.q, pos.r + 1}; break;
                default: return cell;
            }
            return board.isValid(direct) ? std::optional<HexCell>(direct) : std::nullopt;
        }

        return cell;
    }
}


namespace {
    std::string dirName(Board::Direction d) {
        switch (d) {
            case Board::Direction::NORTH: return "NORTH";
            case Board::Direction::SOUTH: return "SOUTH";
            case Board::Direction::EAST: return "EAST";
            case Board::Direction::WEST: return "WEST";
            case Board::Direction::NORTH_EAST: return "NORTH_EAST";
            case Board::Direction::NORTH_WEST: return "NORTH_WEST";
            case Board::Direction::SOUTH_EAST: return "SOUTH_EAST";
            case Board::Direction::SOUTH_WEST: return "SOUTH_WEST";
            default: return "UNKNOWN";
        }
    }

    std::string cellStr(const HexCell& c) {
        return "(" + std::to_string(c.q) + "," + std::to_string(c.r) + ")";
    }
}

std::vector<HexCell> Knight::getMoves(const Board& board) const {
    using Dir = Board::Direction;

    static constexpr std::array<std::pair<Dir, Dir>, 8> kPatterns = {{
        {Dir::NORTH, Dir::EAST},  {Dir::NORTH, Dir::WEST},
        {Dir::SOUTH, Dir::EAST},  {Dir::SOUTH, Dir::WEST},
        {Dir::EAST,  Dir::NORTH}, {Dir::EAST,  Dir::SOUTH},
        {Dir::WEST,  Dir::NORTH}, {Dir::WEST,  Dir::SOUTH},
    }};

    bool isDebug = true; // Enable to debug move filtering

    // Long bras : autorise uniquement même sextant + frontières lisses (S0↔S5, S3↔S4, S5↔S4).
    auto isForbiddenLong = [&board](const HexCell& from, const HexCell& to) -> bool {
        const int fs = board.getSextant(from);
        const int ts = board.getSextant(to);
        if (fs == ts) return false;
        if ((fs == 0 && ts == 5) || (fs == 5 && ts == 0)) return false;
        if ((fs == 3 && ts == 4) || (fs == 4 && ts == 3)) return false;
        if ((fs == 5 && ts == 4) || (fs == 4 && ts == 5)) return false;
        return true;
    };

    // Perpendicular step: allow smooth seam boundaries + selected sextant transitions
    auto isForbiddenPerp = [&board](const HexCell& from, const HexCell& to) -> bool {
        const int fs = board.getSextant(from);
        const int ts = board.getSextant(to);
        if (fs == ts) return false;
        if ((fs == 0 && ts == 5) || (fs == 5 && ts == 0)) return false;
        if ((fs == 3 && ts == 4) || (fs == 4 && ts == 3)) return false;
        // Knight-specific: allow more S5↔S4 transitions than Pawns
        if ((fs == 5 && ts == 4) || (fs == 4 && ts == 5)) return false;
        if (fs == 0 && ts == 1 && from.r == 3)  return false;
        if (fs == 1 && ts == 0 && from.q == 3)  return false;
        if (fs == 2 && ts == 3 && from.r == 7)  return false;
        if (fs == 3 && ts == 2 && from.q == 11) return false;
        return true;
    };

    std::unordered_set<HexCell, HexHash> seen;
    std::vector<HexCell> moves;

    for (auto [d1, d2] : kPatterns) {

        auto a = knightActualStep(board, pos, d1);
        if (!a) {
                continue;
        }
        if (isForbiddenLong(pos, *a)) {
                continue;
        }

        auto b = knightActualStep(board, *a, d1);
        if (!b) {
                continue;
        }
        if (isForbiddenLong(*a, *b)) {
                continue;
        }

        auto c = knightActualStep(board, *b, d2);
        if (!c) {
                continue;
        }
        if (isForbiddenPerp(*b, *c)) {
                continue;
        }

        if (!seen.insert(*c).second) {
                continue;
        }

        Piece* target = board.getPiece(*c);
        if (target && target->getOwner() == owner) {
                continue;
        }

        moves.push_back(*c);
    }


    return moves;
}
