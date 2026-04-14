#include "Bishop.hpp"
#include "Board.hpp"

// Le fou se deplace en diagonale dans 4 directions : NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST.
// Il utilise Board::ray() qui suit la topologie des coutures, ce qui lui permet de traverser
// les 3 zones du plateau sans logique supplementaire.
//
// Difference avec la tour :
//   - Tour  : directions cardinales (NORTH, SOUTH, EAST, WEST)
//   - Fou   : directions diagonales (NE, SW, SE, NW)
// Les deux s'appuient sur ray(), mais traversent des familles de liaisons differentes
// dans Board::buildNeighbors().
//
// Test de reference : fou en {6,2} doit atteindre {8,4} (zone PLAYER2)
// -> couverture assuree par testBishopCrossesIntoOtherZoneDiagonally() dans game_logic_manual_test.cpp
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