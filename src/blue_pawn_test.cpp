#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugBluePawn() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST PION BLEU (3,7) ===\n\n";

    // Cas exact: Pion bleu (PLAYER2) en (3,7)
    std::cout << "Test: Pion bleu PLAYER2 en (3,7)\n";
    
    board.setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 7}));
    // Mettre un ennemi en (4,2) pour voir ce qui se passe
    board.setPiece({4, 2}, factory.create(PieceType::PAWN, Player::PLAYER1, {4, 2}));

    Pawn* pawn = static_cast<Pawn*>(board.getPiece({3, 7}));
    if (pawn) {
        std::vector<HexCell> moves = pawn->getMoves(board);
        std::vector<HexCell> captures = pawn->getCaptureSquares(board, nullptr);
        
        std::cout << "Mouvements normaux:\n";
        for (const auto& move : moves) {
            std::cout << "  - (" << move.q << "," << move.r << ")\n";
        }
        
        std::cout << "Captures:\n";
        for (const auto& cap : captures) {
            Piece* target = board.getPiece(cap);
            std::cout << "  - (" << cap.q << "," << cap.r << ") -> " 
                      << (target ? "ENNEMI" : "VIDE") << "\n";
        }
        
        std::cout << "\nTotal des coups possibles: " << (moves.size() + captures.size()) << "\n";
        
        // Fusionner comme le fait GameState
        std::vector<HexCell> allMoves = moves;
        allMoves.insert(allMoves.end(), captures.begin(), captures.end());
        
        std::cout << "\nTous les coups (fusionnés):\n";
        for (const auto& move : allMoves) {
            Piece* target = board.getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        
        // Vérifier les doublons
        std::cout << "\nVérification des doublons:\n";
        for (size_t i = 0; i < allMoves.size(); ++i) {
            for (size_t j = i + 1; j < allMoves.size(); ++j) {
                if (allMoves[i] == allMoves[j]) {
                    std::cout << "DOUBLON: (" << allMoves[i].q << "," << allMoves[i].r << ")\n";
                }
            }
        }
    }

    std::cout << "\n=== ANALYSE DÉTAILLÉE ===\n";
    
    // Vérifier forward et transition
    std::optional<HexCell> forward = board.step({3, 7}, Board::Direction::EAST);
    std::optional<HexCell> transition = board.getPawnTransition({3, 7}, Player::PLAYER2);
    
    std::cout << "Forward normal: ";
    if (forward) {
        std::cout << "(" << forward->q << "," << forward->r << ")\n";
        Piece* onForward = board.getPiece(*forward);
        std::cout << "Pièce sur forward: " << (onForward ? "OUI" : "NON") << "\n";
    } else {
        std::cout << "AUCUN\n";
    }
    
    std::cout << "Transition: ";
    if (transition) {
        std::cout << "(" << transition->q << "," << transition->r << ")\n";
        Piece* onTransition = board.getPiece(*transition);
        std::cout << "Pièce sur transition: " << (onTransition ? "OUI" : "NON") << "\n";
    } else {
        std::cout << "AUCUNE\n";
    }
    
    std::cout << "\nZone owners:\n";
    std::cout << "(3,7): " << static_cast<int>(board.getZoneOwner({3, 7})) << "\n";
    if (forward) std::cout << "Forward: " << static_cast<int>(board.getZoneOwner(*forward)) << "\n";
    if (transition) std::cout << "Transition: " << static_cast<int>(board.getZoneOwner(*transition)) << "\n";
    std::cout << "(4,2): " << static_cast<int>(board.getZoneOwner({4, 2})) << "\n";
}

int main() {
    debugBluePawn();
    return 0;
}
