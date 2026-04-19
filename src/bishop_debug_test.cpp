#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugBishop() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST PROBLÈME FOU ===\n\n";

    // Tester un fou dans une position typique
    std::cout << "Test: Fou en position standard\n";
    board.setPiece({4, 2}, factory.create(PieceType::BISHOP, Player::PLAYER1, {4, 2}));

    Bishop* bishop = static_cast<Bishop*>(board.getPiece({4, 2}));
    if (bishop) {
        std::vector<HexCell> moves = bishop->getMoves(board);
        
        std::cout << "Mouvements du fou (4,2):\n";
        for (const auto& move : moves) {
            Piece* target = board.getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        std::cout << "Total: " << moves.size() << " mouvements\n\n";
    }

    // Test avec un fou près d'une couture
    std::cout << "Test: Fou près d'une couture\n";
    Board board2; // créer un nouveau board pour ce test
    board2.setPiece({7, 1}, factory.create(PieceType::BISHOP, Player::PLAYER1, {7, 1}));

    bishop = static_cast<Bishop*>(board2.getPiece({7, 1}));
    if (bishop) {
        std::vector<HexCell> moves = bishop->getMoves(board2);
        
        std::cout << "Mouvements du fou (7,1):\n";
        for (const auto& move : moves) {
            Piece* target = board.getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        std::cout << "Total: " << moves.size() << " mouvements\n\n";
    }

    // Test avec des pièces qui bloquent
    std::cout << "Test: Fou avec pièces bloquantes\n";
    Board board3; // créer un nouveau board pour ce test
    board3.setPiece({4, 2}, factory.create(PieceType::BISHOP, Player::PLAYER1, {4, 2}));
    board3.setPiece({2, 4}, factory.create(PieceType::PAWN, Player::PLAYER1, {2, 4})); // pièce alliée
    board3.setPiece({6, 0}, factory.create(PieceType::PAWN, Player::PLAYER2, {6, 0})); // pièce ennemie

    bishop = static_cast<Bishop*>(board3.getPiece({4, 2}));
    if (bishop) {
        std::vector<HexCell> moves = bishop->getMoves(board3);
        
        std::cout << "Mouvements du fou (4,2) avec blocage:\n";
        for (const auto& move : moves) {
            Piece* target = board3.getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
        std::cout << "Total: " << moves.size() << " mouvements\n\n";
    }

    std::cout << "=== ANALYSE ===\n";
    std::cout << "Quel est le problème exact avec le fou ?\n";
    std::cout << "- Mouvements anormaux ?\n";
    std::cout << "- Cases incorrectes ?\n";
    std::cout << "- Problème avec les coutures ?\n";
    std::cout << "- Problème de capture/blocage ?\n";
}

int main() {
    debugBishop();
    return 0;
}
