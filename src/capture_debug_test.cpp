#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugPawnCapture() {
    Board board;
    PieceFactory factory;

    // Cas du pion bleu (PLAYER2) en 3,7
    board.setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 7}));
    
    // Mettre un ennemi sur la case de capture attendue (4,2)
    board.setPiece({4, 2}, factory.create(PieceType::PAWN, Player::PLAYER1, {4, 2}));

    std::cout << "=== DEBUG CAPTURE PION BLEU (3,7) ===\n";
    
    Pawn* pawn = static_cast<Pawn*>(board.getPiece({3, 7}));
    if (!pawn) {
        std::cout << "ERREUR: pion non trouvé en (3,7)\n";
        return;
    }

    // Obtenir les mouvements et captures
    std::vector<HexCell> moves = pawn->getMoves(board);
    std::vector<HexCell> captures = pawn->getCaptureSquares(board, nullptr);

    std::cout << "Mouvements possibles:\n";
    for (const auto& move : moves) {
        std::cout << "  - (" << move.q << "," << move.r << ")\n";
    }

    std::cout << "Captures possibles:\n";
    for (const auto& capture : captures) {
        Piece* target = board.getPiece(capture);
        std::cout << "  - (" << capture.q << "," << capture.r << ") -> " 
                  << (target ? "ENNEMI" : "VIDE") << "\n";
    }

    // Vérifier si 4,2 est dans les captures
    bool foundCapture = false;
    for (const auto& cap : captures) {
        if (cap.q == 4 && cap.r == 2) {
            foundCapture = true;
            break;
        }
    }

    std::cout << "\n=== RÉSULTAT ===\n";
    std::cout << "Capture (4,2) attendue: " << (foundCapture ? "OUI" : "NON") << "\n";
    
    if (!foundCapture) {
        std::cout << "\n=== ANALYSE ===\n";
        
        // Vérifier la couture
        std::optional<HexCell> transition = board.getPawnTransition({3, 7}, Player::PLAYER2);
        std::cout << "Transition (couture): ";
        if (transition) {
            std::cout << "(" << transition->q << "," << transition->r << ")\n";
            Piece* onTransition = board.getPiece(*transition);
            std::cout << "Pièce sur transition: " << (onTransition ? "OUI" : "NON") << "\n";
        } else {
            std::cout << "AUCUNE\n";
        }

        // Vérifier le forward normal
        std::optional<HexCell> forward = board.step({3, 7}, Board::Direction::EAST); // EAST pour PLAYER2 en sextant 1
        std::cout << "Forward normal: ";
        if (forward) {
            std::cout << "(" << forward->q << "," << forward->r << ")\n";
        } else {
            std::cout << "AUCUN\n";
        }

        // Zone owners
        std::cout << "Zone owner (3,7): " << static_cast<int>(board.getZoneOwner({3, 7})) << "\n";
        if (transition) {
            std::cout << "Zone owner transition: " << static_cast<int>(board.getZoneOwner(*transition)) << "\n";
        }
        std::cout << "Zone owner (4,2): " << static_cast<int>(board.getZoneOwner({4, 2})) << "\n";
    }
}

int main() {
    debugPawnCapture();
    return 0;
}
