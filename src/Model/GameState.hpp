#pragma once
#include "Board.hpp"
#include "IObserver.hpp"
#include <vector>

enum class GameStatus {
    PLAYING,
    CHECK,
    CHECKMATE,
    DRAW
};

struct Move {
    HexCell from;
    HexCell to;
    Player  player;
};

class GameState {
public:
    GameState();

    void applyMove(const Move& m);
    void undoMove();
    void nextPlayer();
    int  evaluate() const;

    Board&       getBoard()         { return board; }
    const Board& getBoard()   const { return board; }
    Player       getCurrentPlayer() const { return currentPlayer; }
    GameStatus   getStatus()        const { return status; }

    void addObserver(IObserver* o);
    void notifyAll();

private:
    Board              board;
    Player             currentPlayer;
    GameStatus         status;
    std::vector<Move>  moveHistory;
    std::vector<IObserver*> observers;
};