#include "GameState.hpp"
#include "Pawn.hpp"
#include "PieceFactory.hpp"
#include <array>
#include <cmath>

namespace {
    Board::Direction pawnForward(Player owner) {
        if (owner == Player::PLAYER1) return Board::Direction::SOUTH;
        if (owner == Player::PLAYER2) return Board::Direction::EAST;
        return Board::Direction::NORTH;
    }

    bool isCastlingCandidateMove(Piece* piece, const HexCell& from, const HexCell& to) {
        return piece && piece->getType() == PieceType::KING && from.r == to.r && std::abs(to.q - from.q) == 2;
    }

    bool buildCastlingMove(Piece* piece, const HexCell& from, const HexCell& to, Move& move) {
        if (!isCastlingCandidateMove(piece, from, to)) {
            return false;
        }

        move.isCastling = true;
        move.rookFrom = {to.q > from.q ? 4 : 0, from.r};
        move.rookTo = {from.q + (to.q > from.q ? 1 : -1), from.r};
        return true;
    }
}

GameState::GameState()
    : currentPlayer(Player::PLAYER1)
    , status(GameStatus::PLAYING)
    , lastAttacker(Player::NONE)
    , eliminatedPlayers()
{}

const Move* GameState::getLastMove() const {
    if (moveHistory.empty()) return nullptr;
    return &moveHistory.back();
}

std::vector<Move> GameState::getLegalMovesAsMove(const HexCell& from) {
    Piece* piece = board.getPiece(from);
    if (!piece) {
        return {};

    }

    std::vector<HexCell> candidateMoves;
    if (piece->getType() == PieceType::PAWN) {
        const Pawn* pawn = dynamic_cast<const Pawn*>(piece);
        if (pawn) {
            candidateMoves = pawn->getMoves(board, getLastMove());
            std::vector<HexCell> captureSquares = pawn->getCaptureSquares(board, getLastMove());
            candidateMoves.insert(candidateMoves.end(), captureSquares.begin(), captureSquares.end());
        } else {
            candidateMoves = piece->getMoves(board);
        }
    } else {
        candidateMoves = piece->getMoves(board);
    }

    std::vector<Move> legalMoves;
    legalMoves.reserve(candidateMoves.size());
    const Player player = piece->getOwner();
    const bool startsInCheck = isInCheck(player);

    for (const HexCell& to : candidateMoves) {
        Move simulated{from, to, player};

        const bool isCastling = buildCastlingMove(piece, from, to, simulated);

        if (isCastling && startsInCheck) {
            continue;
        }

        if (isCastling) {
            HexCell through{from.q + (to.q > from.q ? 1 : -1), from.r};
            Move intermediate{from, through, player};
            buildCastlingMove(piece, from, to, intermediate);
            intermediate.isCastling = false;
            intermediate.rookFrom = {0, 0};
            intermediate.rookTo = {0, 0};

            applyMove(intermediate, true);
            const bool crossesAttackedSquare = isInCheck(player);
            undoMove(false);
            if (crossesAttackedSquare) {
                continue;
            }
        }

        applyMove(simulated, true);
        const bool leavesKingInCheck = isInCheck(player);
        undoMove(false);
        if (!leavesKingInCheck) {
            legalMoves.push_back(simulated);

        }
    }

    return legalMoves;
}

std::vector<HexCell> GameState::getLegalMoves(const HexCell& from) {
    std::vector<Move> legalMovesAsMove = getLegalMovesAsMove(from);
    std::vector<HexCell> legalMoves;
    legalMoves.reserve(legalMovesAsMove.size());
    for (const Move& move : legalMovesAsMove) {
        legalMoves.push_back(move.to);
    }
    return legalMoves;
}

int GameState::minimax(int depth, Player rootPlayer) {
    if (depth == 0 || isGameOver()) {
        return evaluate(rootPlayer);
    }

    std::vector<Move> allMoves;
    for (const HexCell& cell : board.allValidCells()) {
        Piece* piece = board.getPiece(cell);
        if (!piece || piece->getOwner() != currentPlayer) {
            continue;
        }
        std::vector<Move> pieceMoves = getLegalMovesAsMove(cell);
        allMoves.insert(allMoves.end(), pieceMoves.begin(), pieceMoves.end());
    }

    if (allMoves.empty()) {
        return evaluate(rootPlayer);
    }

    // For 3-player games, we need to consider coalition dynamics
    // The current player wants to maximize their score, but the other two players
    // may have different interests. We use a modified evaluation that considers
    // the relative strength of all players.
    
    int bestScore = std::numeric_limits<int>::min();
    
    for (const Move& move : allMoves) {
        applyMove(move, true);
        
        // Evaluate from current player's perspective with coalition awareness
        int score = evaluateWithCoalition(rootPlayer);
        undoMove(false);
        
        bestScore = std::max(bestScore, score);
    }

    return bestScore;
}

int GameState::evaluateWithCoalition(Player rootPlayer) const {
    // Enhanced evaluation for 3-player games considering coalitions
    float score = 0.f;
    
    // Get material values for all players
    float player1Value = 0.f, player2Value = 0.f, player3Value = 0.f;
    
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;
        
        float value = static_cast<float>(p->getValue());
        
        if (p->getOwner() == Player::PLAYER1) {
            player1Value += value;
        } else if (p->getOwner() == Player::PLAYER2) {
            player2Value += value;
        } else if (p->getOwner() == Player::PLAYER3) {
            player3Value += value;
        }
    }
    
    // Coalition-aware evaluation
    switch (rootPlayer) {
        case Player::PLAYER1:
            // Player 1 wants to maximize their advantage over Players 2&3
            // But also wants to prevent Players 2&3 from getting too strong
            score = player1Value - 0.7f * player2Value - 0.7f * player3Value;
            // Bonus for keeping other players balanced
            {
                float balance1 = std::abs(player2Value - player3Value);
                score += balance1 * 0.1f; // Small bonus for balance
            }
            break;
            
        case Player::PLAYER2:
            // Player 2 wants to maximize their advantage over Players 1&3
            score = player2Value - 0.7f * player1Value - 0.7f * player3Value;
            {
                float balance2 = std::abs(player1Value - player3Value);
                score += balance2 * 0.1f;
            }
            break;
            
        case Player::PLAYER3:
            // Player 3 wants to maximize their advantage over Players 1&2
            score = player3Value - 0.7f * player1Value - 0.7f * player2Value;
            {
                float balance3 = std::abs(player1Value - player2Value);
                score += balance3 * 0.1f;
            }
            break;
            
        default:
            score = 0.f;
            break;
    }
    
    // Add position bonuses
    score += evaluatePositionBonus(rootPlayer);
    
    return static_cast<int>(score);
}

std::optional<Move> GameState::findBestMove(int depth, Player aiPlayer) {
    if (currentPlayer != aiPlayer) {
        return std::nullopt;
    }

    std::optional<Move> bestMove;
    int bestScore = std::numeric_limits<int>::min();

    for (const HexCell& cell : board.allValidCells()) {
        Piece* piece = board.getPiece(cell);
        if (!piece || piece->getOwner() != aiPlayer) {
            continue;
        }

        std::vector<Move> legalMoves = getLegalMovesAsMove(cell);
        for (const Move& move : legalMoves) {
            applyMove(move, true);
            const int score = minimax(depth - 1, aiPlayer);
            undoMove(false);

            if (!bestMove.has_value() || score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    return bestMove;
}

bool GameState::isGameOver() const {
    return activePlayerCount() <= 1;
}

Player GameState::getWinner() const {
    if (!isGameOver()) {
        return Player::NONE;
    }
    for (Player p : {Player::PLAYER1, Player::PLAYER2, Player::PLAYER3}) {
        if (!isEliminated(p)) {
            return p;
        }
    }
    return Player::NONE;
}

bool GameState::isEliminated(Player p) const {
    return eliminatedPlayers.count(p) > 0;
}

int GameState::activePlayerCount() const {
    int count = 0;
    for (Player p : {Player::PLAYER1, Player::PLAYER2, Player::PLAYER3}) {
        if (!isEliminated(p)) {
            ++count;
        }
    }
    return count;
}

void GameState::computeStatus() {
    bool hasLegal = false;
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p || p->getOwner() != currentPlayer) continue;
        if (!getLegalMoves(c).empty()) {
            hasLegal = true;
            break;
        }
    }

    if (!hasLegal && isInCheck(currentPlayer)) {
        const Player eliminatedPlayer = currentPlayer;
        eliminatedPlayers.insert(eliminatedPlayer);

        if (!moveHistory.empty()) {
            moveHistory.back().yaltaEliminatedPlayer = eliminatedPlayer;
            moveHistory.back().yaltaPiecesOwnerBefore.clear();
        }

        for (const HexCell& c : board.allValidCells()) {
            Piece* p = board.getPiece(c);
            if (p && p->getOwner() == eliminatedPlayer) {
                if (!moveHistory.empty()) {
                    moveHistory.back().yaltaPiecesOwnerBefore.push_back({c, p->getOwner()});
                }
                p->setOwner(lastAttacker);
            }
        }

        nextPlayer();
        if (activePlayerCount() > 1) {
            status = GameStatus::PLAYING;
        } else {
            status = GameStatus::CHECKMATE;
        }
    } else if (!hasLegal) {
        status = GameStatus::DRAW;
    } else if (isInCheck(currentPlayer)) {
        status = GameStatus::CHECK;
    } else {
        status = GameStatus::PLAYING;
    }
}

bool GameState::isInCheck(Player player) const {
    HexCell kingPos{-1, -1};
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);

        if (p && p->getOwner() == player && p->getType() == PieceType::KING) {
            kingPos = c;
            break;
        }
    }
    if (!board.isValid(kingPos)) return false;

    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p || p->getOwner() == player) continue;

        std::vector<HexCell> threats;
        if (p->getType() == PieceType::PAWN) {
            const Pawn* pawn = dynamic_cast<const Pawn*>(p);
            threats = pawn ? pawn->getCaptureSquares(board, getLastMove()) : p->getMoves(board);
        } else {
            threats = p->getMoves(board);
        }
        for (const HexCell& t : threats) {
            if (t == kingPos) return true;
        }
    }
    return false;

}

void GameState::applyMove(const Move& m, bool isSimulation) {
    Move applied = m;
    applied.previousLastAttacker = lastAttacker;
    applied.previousStatus = status;
    applied.yaltaEliminatedPlayer = Player::NONE;
    applied.yaltaPiecesOwnerBefore.clear();
    if (!isSimulation) {
        lastAttacker = m.player;
    }
    Piece* moving = board.getPiece(m.from);
    if (!moving) return;

    applied.movingPieceHadMoved = moving->getHasMoved();

    Piece* capturedPiece = board.getPiece(m.to);
    if (moving->getType() == PieceType::PAWN) {
        const Move* previous = getLastMove();
        if (capturedPiece == nullptr && previous && previous->player != moving->getOwner()) {
            Piece* lastPawn = board.getPiece(previous->to);
            if (lastPawn && lastPawn->getType() == PieceType::PAWN) {
                const Board::Direction enemyForward = pawnForward(lastPawn->getOwner());
                std::optional<HexCell> mid = board.step(previous->from, enemyForward);
                std::optional<HexCell> end = mid.has_value() ? board.step(*mid, enemyForward) : std::nullopt;
                if (mid.has_value() && end.has_value() && *end == previous->to && *mid == m.to) {
                    Piece* cap = board.getPiece(previous->to);
                    if (cap && cap->getOwner() != moving->getOwner()) {
                        applied.isEnPassant = true;
                        applied.capturedPawnCell = previous->to;
                        applied.capturedExists = true;
                        applied.capturedType = cap->getType();
                        applied.capturedOwner = cap->getOwner();
                        applied.capturedCell = previous->to;
                        board.removePiece(previous->to);
                        delete cap;
                    }
                }
            }
        }
    }

    if (capturedPiece) {
        applied.capturedExists = true;
        applied.capturedType = capturedPiece->getType();
        applied.capturedOwner = capturedPiece->getOwner();

        applied.capturedCell = m.to;
        board.removePiece(m.to);
        delete capturedPiece;
    }

    if (applied.isCastling) {
        Piece* rook = board.getPiece(applied.rookFrom);
        if (rook && rook->getType() == PieceType::ROOK && rook->getOwner() == moving->getOwner()) {
            applied.rookHadMoved = rook->getHasMoved();
            board.movePiece(applied.rookFrom, applied.rookTo);
            rook->setHasMoved(true);
        } else {
            applied.isCastling = false;
            applied.rookFrom = {0, 0};
            applied.rookTo = {0, 0};
        }
    }

    board.movePiece(m.from, m.to);
    moving->setHasMoved(true);

    if (moving->getType() == PieceType::PAWN) {
        if (board.isPromotionCell(m.to, moving->getOwner())) {
            applied.isPromotion = true;
            PieceFactory factory;
            Piece* newQ = factory.create(PieceType::QUEEN, moving->getOwner(), m.to);

            board.setPiece(m.to, newQ);
            delete moving;
        }
    }

    moveHistory.push_back(applied);
    nextPlayer();
    if (!isSimulation) {
        computeStatus();
        notifyAll();
    }
}

void GameState::undoMove(bool notifyObservers) {
    if (moveHistory.empty()) return;
    Move last = moveHistory.back();
    moveHistory.pop_back();

    PieceFactory factory;

    if (last.yaltaEliminatedPlayer != Player::NONE) {
        eliminatedPlayers.erase(last.yaltaEliminatedPlayer);
        for (const auto& [cell, owner] : last.yaltaPiecesOwnerBefore) {
            Piece* piece = board.getPiece(cell);
            if (piece) {
                piece->setOwner(owner);
            }
        }
    }

    if (last.isPromotion) {
        Piece* promoted = board.getPiece(last.to);
        if (promoted) {
            board.removePiece(last.to);

            delete promoted;
        }

        Piece* pawn = factory.create(PieceType::PAWN, last.player, last.from);
        if (pawn) {
            pawn->setHasMoved(true);
            board.setPiece(last.from, pawn);
            pawn->setHasMoved(last.movingPieceHadMoved);
        }
    } else {
        board.movePiece(last.to, last.from);
        Piece* moving = board.getPiece(last.from);
        if (moving) {
            moving->setHasMoved(last.movingPieceHadMoved);
        }
    }

    if (last.isCastling) {
        Piece* rook = board.getPiece(last.rookTo);
        if (rook) {
            board.movePiece(last.rookTo, last.rookFrom);
            rook->setHasMoved(last.rookHadMoved);
        }
    }

    if (last.capturedExists && !last.isEnPassant) {
        Piece* restored = factory.create(last.capturedType, last.capturedOwner, last.capturedCell);
        if (restored) {
            board.setPiece(last.capturedCell, restored);
        }
    }

    if (last.isEnPassant) {
        Piece* pawn = factory.create(PieceType::PAWN, last.capturedOwner, last.capturedPawnCell);
        if (pawn) {
            board.setPiece(last.capturedPawnCell, pawn);
        }
    }

    currentPlayer = last.player;
    status = last.previousStatus;
    lastAttacker = last.previousLastAttacker;

    if (notifyObservers) {
        notifyAll();
    }
}

void GameState::nextPlayer() {
    Player next = currentPlayer;
    for (int i = 0; i < 3; ++i) {
        switch (next) {
            case Player::PLAYER1: next = Player::PLAYER2; break;
            case Player::PLAYER2: next = Player::PLAYER3; break;
            case Player::PLAYER3: next = Player::PLAYER1; break;
            default: next = Player::PLAYER1; break;
        }
        if (!isEliminated(next)) {
            currentPlayer = next;
            return;
        }
    }
}

int GameState::evaluate(Player perspective) const {
    float score = 0.f;
    
    // Count pieces by player for material evaluation
    int player1Count = 0, player2Count = 0, player3Count = 0;
    float player1Value = 0.f, player2Value = 0.f, player3Value = 0.f;
    
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;
        
        float value = static_cast<float>(p->getValue());
        
        if (p->getOwner() == Player::PLAYER1) {
            player1Count++;
            player1Value += value;
        } else if (p->getOwner() == Player::PLAYER2) {
            player2Count++;
            player2Value += value;
        } else if (p->getOwner() == Player::PLAYER3) {
            player3Count++;
            player3Value += value;
        }
    }
    
    // Differential scoring based on perspective
    switch (perspective) {
        case Player::PLAYER1:
            score = player1Value - 0.6f * player2Value - 0.6f * player3Value;
            // Position bonus: center control and advancement
            score += evaluatePositionBonus(Player::PLAYER1);
            break;
        case Player::PLAYER2:
            score = player2Value - 0.6f * player1Value - 0.6f * player3Value;
            score += evaluatePositionBonus(Player::PLAYER2);
            break;
        case Player::PLAYER3:
            score = player3Value - 0.6f * player1Value - 0.6f * player2Value;
            score += evaluatePositionBonus(Player::PLAYER3);
            break;
        default:
            score = 0.f;
            break;
    }
    
    return static_cast<int>(score);
}

float GameState::evaluatePositionBonus(Player player) const {
    float bonus = 0.f;
    
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p || p->getOwner() != player) continue;
        
        // Center control bonus
        if (c.q >= 3 && c.q <= 8 && c.r >= 3 && c.r <= 8) {
            bonus += 2.f;
        }
        
        // Advancement bonus (pawns closer to promotion)
        if (p->getType() == PieceType::PAWN) {
            if (player == Player::PLAYER1 && c.r >= 6) bonus += 1.f;
            if (player == Player::PLAYER2 && c.q <= 2) bonus += 1.f;
            if (player == Player::PLAYER3 && c.q >= 9) bonus += 1.f;
        }
        
        // Piece development bonus
        if (p->getHasMoved() == false) {
            bonus += 0.5f;
        }
    }
    
    return bonus;
}

int GameState::evaluate() const {
    return evaluate(currentPlayer);
}

void GameState::addObserver(IObserver* o) {
    observers.push_back(o);
}

void GameState::notifyAll() {
    for (auto o : observers)
        o->onStateChanged();
}