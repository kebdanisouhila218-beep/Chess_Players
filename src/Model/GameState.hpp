#pragma once
#include "Board.hpp"
#include "IObserver.hpp"
#include <optional>
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
    int previousHalfmoveClock = 0;
    Player yaltaEliminatedPlayer = Player::NONE;
    std::vector<std::pair<HexCell, Player>> yaltaPiecesOwnerBefore;
};

class GameState {
public:
    GameState();
    GameState(const GameState& other);
    GameState& operator=(const GameState&) = delete;

    void applyMove(const Move& m, bool isSimulation = false);
    void undoMove(bool notifyObservers = true);
    std::vector<Move> getLegalMovesAsMove(const HexCell& from);
    std::vector<HexCell> getLegalMoves(const HexCell& from);
    void nextPlayer();

    int  minimax(int depth, Player rootPlayer);
    std::optional<Move> findBestMove(int depth, Player aiPlayer);
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
    Player       getLastAttacker()   const { return lastAttacker; }
    int          getHalfmoveClock() const { return halfmoveClock; }
    bool         isInCheck(Player player) const;
    const Move*  getLastMove()      const;
    const std::vector<Move>& getMoveHistory() const { return moveHistory; }

    void addObserver(IObserver* o);
    void notifyAll();

    bool   isPromotionPending()  const { return m_promotionPending; }
    Player getPromotionPlayer()  const { return m_promotionPlayer; }
    void   applyPromotion(PieceType chosen);

private:
    Board              board;
    Player             currentPlayer;
    GameStatus         status;
    std::vector<Move>  moveHistory;
    std::set<Player>   eliminatedPlayers;
    Player             lastAttacker = Player::NONE;
    int                halfmoveClock = 0;
    std::vector<IObserver*> observers;
    bool               m_computingStatus  = false;

    bool               m_promotionPending = false;
    HexCell            m_promotionCell    = {0, 0};
    Player             m_promotionPlayer  = Player::NONE;
    Move               m_promotionApplied;
};