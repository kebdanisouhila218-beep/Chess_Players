#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugLegalMoves() {
    GameState state;
    PieceFactory factory;

    // Cas du pion bleu (PLAYER2) en 3,7
    state.getBoard().setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 7}));
    
    // Mettre un ennemi sur la case de capture attendue (4,2)
    state.getBoard().setPiece({4, 2}, factory.create(PieceType::PAWN, Player::PLAYER1, {4, 2}));

    std::cout << "=== DEBUG LEGAL MOVES PION BLEU (3,7) ===\n";
    
    // Obtenir les coups légaux comme le fait le jeu
    std::vector<HexCell> legalMoves = state.getLegalMoves({3, 7});
    std::vector<Move> legalMovesAsMove = state.getLegalMovesAsMove({3, 7});

    std::cout << "Legal moves (HexCell):\n";
    for (const auto& move : legalMoves) {
        Piece* target = state.getBoard().getPiece(move);
        std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                  << (target ? "PIECE" : "VIDE") << "\n";
    }

    std::cout << "\nLegal moves (Move):\n";
    for (const auto& move : legalMovesAsMove) {
        Piece* target = state.getBoard().getPiece(move.to);
        std::cout << "  - (" << move.from.q << "," << move.from.r << ") -> (" 
                  << move.to.q << "," << move.to.r << ") -> " 
                  << (target ? "PIECE" : "VIDE") << "\n";
    }

    // Vérifier si 4,2 est dans les coups légaux
    bool foundCapture = false;
    for (const auto& move : legalMoves) {
        if (move.q == 4 && move.r == 2) {
            foundCapture = true;
            break;
        }
    }

    std::cout << "\n=== RÉSULTAT ===\n";
    std::cout << "Capture (4,2) dans legal moves: " << (foundCapture ? "OUI" : "NON") << "\n";
    
    if (!foundCapture) {
        std::cout << "\n=== ANALYSE DÉTAILLÉE ===\n";
        
        Pawn* pawn = static_cast<Pawn*>(state.getBoard().getPiece({3, 7}));
        if (pawn) {
            std::vector<HexCell> moves = pawn->getMoves(state.getBoard(), state.getLastMove());
            std::vector<HexCell> captures = pawn->getCaptureSquares(state.getBoard(), state.getLastMove());
            
            std::cout << "Mouvements bruts:\n";
            for (const auto& move : moves) {
                std::cout << "  - (" << move.q << "," << move.r << ")\n";
            }
            
            std::cout << "Captures brutes:\n";
            for (const auto& cap : captures) {
                Piece* target = state.getBoard().getPiece(cap);
                std::cout << "  - (" << cap.q << "," << cap.r << ") -> " 
                          << (target ? "ENNEMI" : "VIDE") << "\n";
            }
            
            std::cout << "Fusion moves+captures:\n";
            std::vector<HexCell> fused = moves;
            fused.insert(fused.end(), captures.begin(), captures.end());
            for (const auto& cell : fused) {
                std::cout << "  - (" << cell.q << "," << cell.r << ")\n";
            }
        }
    }
}

int main() {
    debugLegalMoves();
    return 0;
}
