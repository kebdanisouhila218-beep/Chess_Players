#include "Model/Board.hpp"
#include "Model/Piece.hpp"
#include "Model/PieceFactory.hpp"
#include "Model/GameState.hpp"
#include <iostream>

void debugDoubleMove() {
    Board board;
    PieceFactory factory;

    std::cout << "=== TEST DOUBLE MOUVEMENT PION FRONTIÈRE ===\n\n";

    // Test: Pion blanc en (7,1) avec transition (5,11) occupée par ennemi
    std::cout << "Test: Pion blanc (7,1) avec ennemi sur transition (5,11)\n";
    
    board.setPiece({7, 1}, factory.create(PieceType::PAWN, Player::PLAYER1, {7, 1}));
    board.setPiece({5, 11}, factory.create(PieceType::PAWN, Player::PLAYER3, {5, 11}));

    Pawn* pawn = static_cast<Pawn*>(board.getPiece({7, 1}));
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
        
        // Vérifier s'il y a des doublons
        std::vector<HexCell> allMoves = moves;
        allMoves.insert(allMoves.end(), captures.begin(), captures.end());
        
        std::cout << "Tous les coups (avec doublons possibles):\n";
        for (const auto& move : allMoves) {
            std::cout << "  - (" << move.q << "," << move.r << ")\n";
        }
        
        // Vérifier les doublons
        bool hasDuplicates = false;
        for (size_t i = 0; i < allMoves.size(); ++i) {
            for (size_t j = i + 1; j < allMoves.size(); ++j) {
                if (allMoves[i] == allMoves[j]) {
                    hasDuplicates = true;
                    std::cout << "DOUBLON TROUVÉ: (" << allMoves[i].q << "," << allMoves[i].r << ")\n";
                }
            }
        }
        
        if (!hasDuplicates) {
            std::cout << "Aucun doublon trouvé\n";
        }
    }

    std::cout << "\n=== TEST GETLEGALMOVES ===\n";
    GameState state;
    state.getBoard().setPiece({7, 1}, factory.create(PieceType::PAWN, Player::PLAYER1, {7, 1}));
    state.getBoard().setPiece({5, 11}, factory.create(PieceType::PAWN, Player::PLAYER3, {5, 11}));
    
    std::vector<HexCell> legalMoves = state.getLegalMoves({7, 1});
    std::cout << "Legal moves retournés par GameState:\n";
    for (const auto& move : legalMoves) {
        Piece* target = state.getBoard().getPiece(move);
        std::cout << "  - (" << move.q << "," << move.r << ") -> " 
                  << (target ? "PIECE" : "VIDE") << "\n";
    }
    std::cout << "Total: " << legalMoves.size() << " coups\n";
}

int main() {
    debugDoubleMove();
    return 0;
}
