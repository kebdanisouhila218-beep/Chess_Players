#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugDetailedFrontier() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST DÉTAILLÉ FRONTIÈRE ===\n\n";

    // Test: Pion blanc en (7,1) vers transition (5,11) occupée par ennemi
    std::cout << "Configuration: Pion blanc (7,1) -> transition (5,11) occupée par ennemi\n";
    
    board.setPiece({7, 1}, factory.create(PieceType::PAWN, Player::PLAYER1, {7, 1}));
    board.setPiece({5, 11}, factory.create(PieceType::PAWN, Player::PLAYER3, {5, 11}));

    // Vérifier la transition
    std::optional<HexCell> transition = board.getPawnTransition({7, 1}, Player::PLAYER1);
    std::cout << "Transition calculée: ";
    if (transition) {
        std::cout << "(" << transition->q << "," << transition->r << ")\n";
        Piece* onTransition = board.getPiece(*transition);
        std::cout << "Pièce sur transition: " << (onTransition ? "OUI" : "NON") << "\n";
        if (onTransition) {
            std::cout << "Propriétaire: " << static_cast<int>(onTransition->getOwner()) << "\n";
            std::cout << "Est ennemi: " << (onTransition->getOwner() != Player::PLAYER1 ? "OUI" : "NON") << "\n";
        }
    } else {
        std::cout << "AUCUNE\n";
    }

    // Vérifier forward normal
    std::optional<HexCell> front = board.step({7, 1}, Board::Direction::SOUTH); // SOUTH pour PLAYER1
    std::cout << "Forward normal: ";
    if (front) {
        std::cout << "(" << front->q << "," << front->r << ")\n";
    } else {
        std::cout << "AUCUN\n";
    }

    Pawn* pawn = static_cast<Pawn*>(board.getPiece({7, 1}));
    if (pawn) {
        std::vector<HexCell> moves = pawn->getMoves(board);
        std::vector<HexCell> captures = pawn->getCaptureSquares(board, nullptr);
        
        std::cout << "\nMouvements possibles:\n";
        for (const auto& move : moves) {
            std::cout << "  - (" << move.q << "," << move.r << ")\n";
        }
        
        std::cout << "\nCaptures possibles:\n";
        for (const auto& cap : captures) {
            Piece* target = board.getPiece(cap);
            std::cout << "  - (" << cap.q << "," << cap.r << ") -> " 
                      << (target ? "ENNEMI" : "VIDE") << "\n";
        }
        
        // Vérifier si la transition elle-même est dans les captures
        bool found = false;
        for (const auto& cap : captures) {
            if (cap.q == 5 && cap.r == 11) {
                found = true;
                break;
            }
        }
        std::cout << "\nTransition (5,11) dans captures: " << (found ? "OUI" : "NON") << "\n";
    }

    std::cout << "\n=== DÉBOGAGE CODE ===\n";
    std::cout << "Dans Pawn::getCaptureSquares():\n";
    std::cout << "1. transition.has_value(): " << (transition.has_value() ? "true" : "false") << "\n";
    if (transition) {
        Piece* transitionPiece = board.getPiece(*transition);
        std::cout << "2. transitionPiece != nullptr: " << (transitionPiece != nullptr ? "true" : "false") << "\n";
        if (transitionPiece) {
            std::cout << "3. transitionPiece->getOwner() != owner: " << (transitionPiece->getOwner() != Player::PLAYER1 ? "true" : "false") << "\n";
        }
    }
}

int main() {
    debugDetailedFrontier();
    return 0;
}
