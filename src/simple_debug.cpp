#include "Model/GameState.hpp"
#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include <iostream>

int main() {
    GameState state;
    
    std::cout << "=== DEBUG SIMPLE ===\n";
    
    // Test direct : getLegalMoves sur une pièce
    HexCell testPos = {5, 1}; // Pion blanc
    
    Piece* p = state.getBoard().getPiece(testPos);
    if (p) {
        std::cout << "Pièce trouvée en " << testPos.q << "," << testPos.r 
                  << " type=" << (int)p->getType() 
                  << " owner=" << (int)p->getOwner() << "\n";
        
        auto moves = state.getLegalMoves(testPos);
        std::cout << "Nombre de coups : " << moves.size() << "\n";
        
        for (auto& move : moves) {
            std::cout << "  -> " << move.q << "," << move.r << "\n";
        }
    } else {
        std::cout << "Pas de pièce en " << testPos.q << "," << testPos.r << "\n";
    }
    
    return 0;
}
