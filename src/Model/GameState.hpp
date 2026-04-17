#pragma once
#include "Board.hpp"
#include "IObserver.hpp"
#include <vector>
#include <set>

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
    Player previousLastAttacker = Player::NONE;
    GameStatus previousStatus = GameStatus::PLAYING;
    Player yaltaEliminatedPlayer = Player::NONE;
    std::vector<std::pair<HexCell, Player>> yaltaPiecesOwnerBefore;
};

class GameState {
public:
    GameState();

    void applyMove(const Move& m, bool isSimulation = false);
    void undoMove();
    std::vector<Move> getLegalMovesAsMove(const HexCell& from);
    std::vector<HexCell> getLegalMoves(const HexCell& from);
    void nextPlayer();
    int  evaluate(Player perspective) const;
    int  evaluate() const;
    bool isGameOver() const;
    Player getWinner() const;

    bool isEliminated(Player p) const;
    int  activePlayerCount() const;
    void computeStatus();

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
    std::set<Player>   eliminatedPlayers;
    Player             lastAttacker = Player::NONE;
    std::vector<IObserver*> observers;
};