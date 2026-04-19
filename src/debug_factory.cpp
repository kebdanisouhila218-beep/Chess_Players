#include "Model/GameState.hpp"
#include "Model/Board.hpp"
#include "Model/PieceFactory.hpp"
#include <iostream>

int main() {
    Board board;
    PieceFactory factory;
    
    std::cout << "=== DEBUG FACTORY ===\n";
    
    // Tester l'initialisation pour chaque joueur
    for (Player p : {Player::PLAYER1, Player::PLAYER2, Player::PLAYER3}) {
        std::cout << "Initialisation joueur " << static_cast<int>(p) << " :\n";
        
        factory.initBoard(board, p);
        
        // Compter les pièces après initialisation
        int count = 0;
        for (const HexCell& cell : board.allValidCells()) {
            if (board.getPiece(cell)) {
                count++;
            }
        }
        
        std::cout << "  Pièces placées : " << count << "\n";
    }
    
    return 0;
}
