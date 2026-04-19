#include "Model/GameState.hpp"
#include "Model/PieceFactory.hpp"

void printBoardVisualLayout(const Board& board);

int main() {
    GameState state;
    PieceFactory factory;

    factory.initBoard(state.getBoard(), Player::PLAYER1);
    factory.initBoard(state.getBoard(), Player::PLAYER2);
    factory.initBoard(state.getBoard(), Player::PLAYER3);

    printBoardVisualLayout(state.getBoard());
    return 0;
}
