#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugBluePawnWithEnemy() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST PION BLEU (3,7) AVEC ENNEMI EN (4,2) ===\n\n";

    // Cas exact: Pion bleu (PLAYER2) en (3,7) avec ennemi en (4,2)
    board.setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 7}));
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
        
        std::cout << "\nFusion moves + captures:\n";
        std::vector<HexCell> allMoves = moves;
        allMoves.insert(allMoves.end(), captures.begin(), captures.end());
        for (const auto& move : allMoves) {
            Piece* target = board.getPiece(move);
            std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                      << (target ? "PIECE" : "VIDE") << "\n";
        }
    }

    std::cout << "\n=== TEST AVEC GAMESTATE ===\n";
    GameState state;
    state.getBoard().setPiece({3, 7}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 7}));
    state.getBoard().setPiece({4, 2}, factory.create(PieceType::PAWN, Player::PLAYER1, {4, 2}));
    
    std::vector<HexCell> legalMoves = state.getLegalMoves({3, 7});
    std::cout << "Legal moves retournés par GameState:\n";
    for (const auto& move : legalMoves) {
        Piece* target = state.getBoard().getPiece(move);
        std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                  << (target ? "PIECE" : "VIDE") << "\n";
    }
    std::cout << "Total: " << legalMoves.size() << " coups\n";
    
    // Vérifier spécifiquement si (4,2) est là
    bool found42 = false;
    for (const auto& move : legalMoves) {
        if (move.q == 4 && move.r == 2) {
            found42 = true;
            break;
        }
    }
    std::cout << "(4,2) trouvé dans legal moves: " << (found42 ? "OUI" : "NON") << "\n";
}

int main() {
    debugBluePawnWithEnemy();
    return 0;
}
