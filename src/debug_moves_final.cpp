#include "Model/GameState.hpp"
#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include <iostream>

int main() {
    GameState state;
    
    std::cout << "=== DEBUG FINAL : TOUS LES JOUEURS ===\n\n";
    
    // Tester tous les joueurs
    for (Player p : {Player::PLAYER1, Player::PLAYER2, Player::PLAYER3}) {
        std::cout << "JOUEUR " << static_cast<int>(p) << " :\n";
        
        int totalPieces = 0;
        int piecesWithMoves = 0;
        
        for (const HexCell& cell : state.getBoard().allValidCells()) {
            Piece* piece = state.getBoard().getPiece(cell);
            if (!piece || piece->getOwner() != p) continue;
            
            totalPieces++;
            auto moves = state.getLegalMoves(cell);
            
            if (!moves.empty()) {
                piecesWithMoves++;
                std::cout << "  Pièce " << static_cast<int>(piece->getType()) 
                          << " en " << cell.q << "," << cell.r 
                          << " a " << moves.size() << " coups\n";
            }
        }
        
        std::cout << "  Total pièces : " << totalPieces << "\n";
        std::cout << "  Pièces avec mouvements : " << piecesWithMoves << "\n\n";
    }
    
    return 0;
}
