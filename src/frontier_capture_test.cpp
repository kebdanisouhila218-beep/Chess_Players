#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugFrontierCapture() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST CAPTURE À LA FRONTIÈRE ===\n\n";

    // Test 1: Pion qui veut traverser vers une case occupée par ennemi
    std::cout << "Test 1: Pion vers case de transition occupée par ennemi\n";
    board.setPiece({7, 1}, factory.create(PieceType::PAWN, Player::PLAYER1, {7, 1}));
    board.setPiece({4, 11}, factory.create(PieceType::PAWN, Player::PLAYER3, {4, 11}));

    Pawn* pawn1 = static_cast<Pawn*>(board.getPiece({7, 1}));
    if (pawn1) {
        std::vector<HexCell> moves = pawn1->getMoves(board);
        std::vector<HexCell> captures = pawn1->getCaptureSquares(board, nullptr);
        
        std::cout << "Mouvements: ";
        for (const auto& move : moves) {
            std::cout << "(" << move.q << "," << move.r << ") ";
        }
        std::cout << "\n";
        
        std::cout << "Captures: ";
        for (const auto& cap : captures) {
            Piece* target = board.getPiece(cap);
            std::cout << "(" << cap.q << "," << cap.r << ")->" << (target ? "ENNEMI" : "VIDE") << " ";
        }
        std::cout << "\n\n";
    }

    // Test 2: Pion qui veut avancer normalement vers case occupée par ennemi
    std::cout << "Test 2: Pion vers case normale occupée par ennemi\n";
    board.setPiece({5, 5}, factory.create(PieceType::PAWN, Player::PLAYER1, {5, 5}));
    board.setPiece({5, 4}, factory.create(PieceType::PAWN, Player::PLAYER2, {5, 4}));

    Pawn* pawn2 = static_cast<Pawn*>(board.getPiece({5, 5}));
    if (pawn2) {
        std::vector<HexCell> moves = pawn2->getMoves(board);
        std::vector<HexCell> captures = pawn2->getCaptureSquares(board, nullptr);
        
        std::cout << "Mouvements: ";
        for (const auto& move : moves) {
            std::cout << "(" << move.q << "," << move.r << ") ";
        }
        std::cout << "\n";
        
        std::cout << "Captures: ";
        for (const auto& cap : captures) {
            Piece* target = board.getPiece(cap);
            std::cout << "(" << cap.q << "," << cap.r << ")->" << (target ? "ENNEMI" : "VIDE") << " ";
        }
        std::cout << "\n\n";
    }

    // Test 3: Cas du pion bleu en 3,7 avec ennemi en 3,3
    std::cout << "Test 3: Pion bleu (3,7) avec ennemi en (3,3)\n";
    board.setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 3}));
    board.setPiece({3, 3}, factory.create(PieceType::PAWN, Player::PLAYER1, {3, 3}));

    Pawn* pawn3 = static_cast<Pawn*>(board.getPiece({3, 7}));
    if (pawn3) {
        std::vector<HexCell> moves = pawn3->getMoves(board);
        std::vector<HexCell> captures = pawn3->getCaptureSquares(board, nullptr);
        
        std::cout << "Mouvements: ";
        for (const auto& move : moves) {
            std::cout << "(" << move.q << "," << move.r << ") ";
        }
        std::cout << "\n";
        
        std::cout << "Captures: ";
        for (const auto& cap : captures) {
            Piece* target = board.getPiece(cap);
            std::cout << "(" << cap.q << "," << cap.r << ")->" << (target ? "ENNEMI" : "VIDE") << " ";
        }
        std::cout << "\n\n";
    }

    std::cout << "=== ANALYSE ===\n";
    std::cout << "Problème: Si la case de destination est occupée,\n";
    std::cout << "le pion devrait pouvoir capturer si c'est un ennemi,\n";
    std::cout << "pas être complètement bloqué.\n";
}

int main() {
    debugFrontierCapture();
    return 0;
}
