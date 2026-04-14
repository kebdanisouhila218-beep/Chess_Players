#include "Queen.hpp"
#include "Board.hpp"
#include <algorithm>
#include <unordered_set>

// La dame combine les 8 directions : 4 cardinales (tour) + 4 diagonales (fou).
// Elle utilise Board::ray() pour chaque direction, ce qui couvre les coutures.
//
// La deduplication (sort + unique) est necessaire car sur ce plateau torique,
// deux directions distinctes peuvent theoriquement converger vers la meme case
// apres traversee de couture.
//
// Pourquoi pas de getMoves(Board, lastMove) comme le pion ?
// La dame depend uniquement de la geometrie courante du plateau et de
// l'occupation des cases. Elle n'a aucun mouvement conditionnel a l'historique
// (pas d'en passant, pas de double pas initial, pas de transition de couture
// speciale au premier coup). Le pion seul necessite lastMove.
//
// Test de reference : dame en {7,0} doit atteindre {4,11} via couture SOUTH
// -> couverture assuree par testQueenCrossesSeamThroughRay() dans game_logic_manual_test.cpp
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