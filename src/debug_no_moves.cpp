#include "Model/GameState.hpp"
#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include <iostream>

int main() {
    GameState state;
    
    std::cout << "=== DEBUG : POURQUOI AUCUN COUP PROPOSÉ ===\n\n";
    
    // Tester toutes les pièces du joueur actuel
    Player current = state.getCurrentPlayer();
    std::cout << "Joueur actuel : " << static_cast<int>(current) << "\n";
    
    int totalPieces = 0;
    int piecesWithMoves = 0;
    
    for (const HexCell& cell : state.getBoard().allValidCells()) {
        Piece* p = state.getBoard().getPiece(cell);
        if (!p || p->getOwner() != current) continue;
        
        totalPieces++;
        auto moves = state.getLegalMoves(cell);
        
        if (!moves.empty()) {
            piecesWithMoves++;
            std::cout << "Pièce en " << cell.q << "," << cell.r 
                      << " (" << (int)p->getType() << ") a " << moves.size() << " coups\n";
        } else {
            std::cout << "Pièce en " << cell.q << "," << cell.r 
                      << " (" << (int)p->getType() << ") a 0 coups\n";
        }
    }
    
    std::cout << "\nTotal pièces : " << totalPieces << "\n";
    std::cout << "Pièces avec mouvements : " << piecesWithMoves << "\n";
    
    if (piecesWithMoves == 0) {
        std::cout << "\n!!! CRITIQUE : Aucune pièce ne peut bouger !!!\n";
        
        // Testons un pion spécifiquement
        std::cout << "\nTest d'un pion blanc en (5,1) :\n";
        Piece* testPawn = state.getBoard().getPiece({5, 1});
        if (testPawn && testPawn->getOwner() == Player::PLAYER1) {
            auto pawnMoves = testPawn->getMoves(state.getBoard());
            auto pawnCaptures = testPawn->getCaptureSquares(state.getBoard());
            
            std::cout << "Mouvements du pion : " << pawnMoves.size() << "\n";
            for (auto& move : pawnMoves) {
                std::cout << "  -> " << move.q << "," << move.r << "\n";
            }
            
            std::cout << "Captures du pion : " << pawnCaptures.size() << "\n";
            for (auto& capture : pawnCaptures) {
                std::cout << "  -> " << capture.q << "," << capture.r << "\n";
            }
        } else {
            std::cout << "Pas de pion trouvé en (5,1)\n";
        }
    }
    
    return 0;
}
