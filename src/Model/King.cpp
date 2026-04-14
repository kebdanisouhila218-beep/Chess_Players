#include "King.hpp"
#include "Board.hpp"

// Le roi se deplace d'une case dans les 8 directions via Board::step().
// Il ne peut pas capturer ses propres pieces.
//
// LIMITATION ACTUELLE : getMoves() ne valide pas encore le roque.
// Le roque est partiellement code dans GameState::applyMove() (detection
// d'un deplacement roi de 2 cases en X + deplacement de la tour associee),
// mais sans verification des conditions legales.
//
// Conditions manquantes pour un roque legal sur ce plateau :
//   1. Roi et tour n'ont jamais bouge (hasMoved == false)
//   2. Cases intermediaires entre roi et tour libres
//   3. Roi pas en echec au depart
//   4. Roi ne traverse pas une case attaquee
//   5. Roi n'arrive pas sur une case attaquee
//   6. Identifier quelles tours sont eligibles par joueur/zone
//   7. En 3 joueurs : menaces venant de deux camps ennemis simultanement
//   8. Comportement aux coutures si le chemin du roque traverse une topologie non triviale
//
// isInCheck() n'est pas encore implemente -- voir TODO ci-dessous.
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