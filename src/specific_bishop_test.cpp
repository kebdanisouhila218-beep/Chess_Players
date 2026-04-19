#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugSpecificBishop() {
    std::cout << "=== TEST SPÉCIFIQUE FOU ===\n\n";

    // Tester la position exacte où tu vois le problème
    GameState state;
    PieceFactory factory;
    
    // Initialiser une position de jeu standard
    state.getBoard().setPiece({4, 2}, factory.create(PieceType::BISHOP, Player::PLAYER1, {4, 2}));
    
    std::cout << "Test: Fou PLAYER1 en (4,2) dans position de jeu\n";
    
    Bishop* bishop = static_cast<Bishop*>(state.getBoard().getPiece({4, 2}));
    if (bishop) {
        std::vector<HexCell> moves = bishop->getMoves(state.getBoard());
        std::vector<HexCell> legalMoves = state.getLegalMoves({4, 2});
        
        std::cout << "Mouvements bruts du fou:\n";
        for (const auto& move : moves) {
            Piece* target = state.getBoard().getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        std::cout << "Total bruts: " << moves.size() << "\n\n";
        
        std::cout << "Legal moves retournés par GameState:\n";
        for (const auto& move : legalMoves) {
            Piece* target = state.getBoard().getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        std::cout << "Total légaux: " << legalMoves.size() << "\n\n";
        
        // Vérifier s'il y a des mouvements étranges
        std::cout << "=== VÉRIFICATION DES MOUVEMENTS ===\n";
        for (const auto& move : moves) {
            // Vérifier si le mouvement est dans une zone normale
            int sextant = state.getBoard().getSextant(move);
            Player zoneOwner = state.getBoard().getZoneOwner(move);
            std::cout << "Case (" << move.q << "," << move.r << "): ";
            std::cout << "sextant=" << sextant << ", ";
            std::cout << "zone=" << static_cast<int>(zoneOwner) << "\n";
        }
    }

    std::cout << "\n=== TEST AVEC PIÈCES AUTOUR ===\n";
    
    // Test avec des pièces autour du fou
    state.getBoard().setPiece({5, 1}, factory.create(PieceType::PAWN, Player::PLAYER2, {5, 1}));
    state.getBoard().setPiece({3, 3}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 3}));
    
    bishop = static_cast<Bishop*>(state.getBoard().getPiece({4, 2}));
    if (bishop) {
        std::vector<HexCell> moves = bishop->getMoves(state.getBoard());
        
        std::cout << "Mouvements avec pièces ennemies autour:\n";
        for (const auto& move : moves) {
            Piece* target = state.getBoard().getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE(" + std::to_string(static_cast<int>(target->getOwner())) + ")" : "VIDE") << "\n";
        }
        std::cout << "Total: " << moves.size() << "\n";
    }

    std::cout << "\n=== QUESTION POUR TOI ===\n";
    std::cout << "Quel est exactement le problème que tu vois avec le fou ?\n";
    std::cout << "- Le fou va sur des cases impossibles ?\n";
    std::cout << "- Le fou manque des mouvements ?\n";
    std::cout << "- Le franchit des coutures incorrectement ?\n";
    std::cout << "- Problème de capture ?\n";
    std::cout << "Donne-moi un exemple précis si possible.\n";
}

int main() {
    debugSpecificBishop();
    return 0;
}
