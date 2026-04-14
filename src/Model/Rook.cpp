#include "Rook.hpp"
#include "Board.hpp"

// La tour se deplace en 4 directions cardinales : NORTH, SOUTH, EAST, WEST.
// Elle utilise Board::ray() qui suit la topologie des coutures, ce qui lui permet
// de traverser les 3 zones du plateau sans logique supplementaire.
//
// getMoves(const Board&) suffit car la tour ne depend que de la position
// actuelle des pieces sur le plateau, sans contexte historique.
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