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

    bool   isCastling = false;
    HexCell rookFrom  = {0, 0};
    HexCell rookTo    = {0, 0};
    bool   isEnPassant = false;
    HexCell capturedPawnCell = {0, 0};
    bool   isPromotion = false;
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

    const Move* getLastMove() const;

private:
    Board              board;
    Player             currentPlayer;
    GameStatus         status;
    std::vector<Move>  moveHistory;
    std::vector<IObserver*> observers;
};