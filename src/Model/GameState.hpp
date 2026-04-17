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
    PieceType capturedType = PieceType::PAWN;
    Player capturedOwner = Player::NONE;
    HexCell capturedCell = {0, 0};
    bool capturedExists = false;
    bool movingPieceHadMoved = false;

    bool   isCastling = false;
    HexCell rookFrom  = {0, 0};
    HexCell rookTo    = {0, 0};
    bool rookHadMoved = false;
    bool   isEnPassant = false;
    HexCell capturedPawnCell = {0, 0};
    bool   isPromotion = false;
};

class GameState {
public:
    GameState();

    void applyMove(const Move& m);
    void undoMove();
    std::vector<HexCell> getLegalMoves(const HexCell& from);
    void nextPlayer();
    int  evaluate() const;
    bool isGameOver() const;
    Player getWinner() const;

    Board&       getBoard()         { return board; }
    const Board& getBoard()   const { return board; }
    Player       getCurrentPlayer() const { return currentPlayer; }
    GameStatus   getStatus()        const { return status; }
    bool         isInCheck(Player player) const;
    const Move*  getLastMove()      const;

    void addObserver(IObserver* o);
    void notifyAll();

private:
    Board              board;
    Player             currentPlayer;
    GameStatus         status;
    std::vector<Move>  moveHistory;
    std::vector<IObserver*> observers;
};