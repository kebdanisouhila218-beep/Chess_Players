#include "Model/GameState.hpp"
#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include <iostream>

int main() {
    GameState state;
    
    std::cout << "=== DEBUG BOARD INIT ===\n";
    
    // Vérifier si le plateau a des pièces
    int totalPieces = 0;
    for (const HexCell& cell : state.getBoard().allValidCells()) {
        Piece* p = state.getBoard().getPiece(cell);
        if (p) {
            totalPieces++;
            std::cout << "Pièce " << static_cast<int>(p->getType()) 
                      << " en " << cell.q << "," << cell.r 
                      << " owner=" << static_cast<int>(p->getOwner()) << "\n";
        }
    }
    
    std::cout << "Total pièces sur plateau : " << totalPieces << "\n";
    
    // Vérifier le joueur actuel
    std::cout << "Joueur actuel : " << static_cast<int>(state.getCurrentPlayer()) << "\n";
    
    return 0;
}
