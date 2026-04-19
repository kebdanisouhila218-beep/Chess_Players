#include "Model/GameState.hpp"
#include <iostream>

int main() {
    GameState state;
    
    std::cout << "=== DEBUG GAMESTATE ===\n";
    
    // Vérifier si le plateau a des pièces
    int totalPieces = 0;
    for (const HexCell& cell : state.getBoard().allValidCells()) {
        Piece* p = state.getBoard().getPiece(cell);
        if (p) {
            totalPieces++;
        }
    }
    
    std::cout << "Total pièces sur plateau : " << totalPieces << "\n";
    std::cout << "Joueur actuel : " << static_cast<int>(state.getCurrentPlayer()) << "\n";
    
    // Tester si on peut obtenir des mouvements
    for (const HexCell& cell : state.getBoard().allValidCells()) {
        Piece* p = state.getBoard().getPiece(cell);
        if (p && p->getOwner() == state.getCurrentPlayer()) {
            auto moves = state.getLegalMoves(cell);
            if (!moves.empty()) {
                std::cout << "Pièce en " << cell.q << "," << cell.r 
                          << " a " << moves.size() << " coups\n";
                return 0; // On a trouvé au moins un mouvement
            }
        }
    }
    
    std::cout << "Aucun mouvement trouvé !\n";
    return 1;
}
