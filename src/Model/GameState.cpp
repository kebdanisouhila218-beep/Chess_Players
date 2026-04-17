#include "GameState.hpp"
#include "Pawn.hpp"
#include "PieceFactory.hpp"
#include <cmath>

namespace {
    Board::Direction pawnForward(Player owner) {
        if (owner == Player::PLAYER1) return Board::Direction::SOUTH;
        if (owner == Player::PLAYER2) return Board::Direction::EAST;
        return Board::Direction::NORTH;
    }
}

GameState::GameState()
    : currentPlayer(Player::PLAYER1)
    , status(GameStatus::PLAYING)
{}

const Move* GameState::getLastMove() const {
    if (moveHistory.empty()) return nullptr;
    return &moveHistory.back();
}

std::vector<HexCell> GameState::getLegalMoves(const HexCell& from) {
    Piece* piece = board.getPiece(from);
    if (!piece) {
        return {};
    }

    std::vector<HexCell> candidateMoves;
    if (piece->getType() == PieceType::PAWN) {
        const Pawn* pawn = dynamic_cast<const Pawn*>(piece);
        candidateMoves = pawn ? pawn->getMoves(board, getLastMove()) : piece->getMoves(board);
    } else {
        candidateMoves = piece->getMoves(board);
    }

    std::vector<HexCell> legalMoves;
    legalMoves.reserve(candidateMoves.size());
    const Player player = piece->getOwner();

    for (const HexCell& to : candidateMoves) {
        Move simulated{from, to, player};
        applyMove(simulated);
        const bool leavesKingInCheck = isInCheck(player);
        undoMove();
        if (!leavesKingInCheck) {
            legalMoves.push_back(to);
        }
    }

    return legalMoves;
}

bool GameState::isGameOver() const {
    int kingsAlive = 0;
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (p && p->getType() == PieceType::KING) {
            ++kingsAlive;
        }
    }
    return kingsAlive <= 1;
}

Player GameState::getWinner() const {
    if (!isGameOver()) {
        return Player::NONE;
    }
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (p && p->getType() == PieceType::KING) {
            return p->getOwner();
        }
    }
    return Player::NONE;
}

// ATTENTION : pour les pions, getMoves() retourne les cases de deplacement
// ET de capture melangees. isInCheck() peut donc produire un faux positif
// si un pion est devant le roi sans pouvoir le capturer diagonalement.
// A corriger quand Pawn exposera separement ses cases de capture.
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

        std::vector<HexCell> threats = p->getMoves(board);
        for (const HexCell& t : threats) {
            if (t == kingPos) return true;
        }
    }
    return false;
}

void GameState::applyMove(const Move& m) {
    Move applied = m;
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

    if (moving->getType() == PieceType::KING) {
        const int dx = m.to.q - m.from.q;
        const int dy = m.to.r - m.from.r;
        if (std::abs(dx) == 2 && dy == 0) {
            HexCell rookFrom = {dx > 0 ? 4 : 0, m.from.r};
            HexCell rookTo = {m.from.q + (dx > 0 ? 1 : -1), m.from.r};
            Piece* rook = board.getPiece(rookFrom);
            if (rook && rook->getType() == PieceType::ROOK && rook->getOwner() == moving->getOwner()) {
                applied.isCastling = true;
                applied.rookFrom = rookFrom;
                applied.rookTo = rookTo;
                applied.rookHadMoved = rook->getHasMoved();
                board.movePiece(rookFrom, rookTo);
                rook->setHasMoved(true);
            }
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
    notifyAll();
}

void GameState::undoMove() {
    if (moveHistory.empty()) return;
    Move last = moveHistory.back();
    moveHistory.pop_back();

    PieceFactory factory;

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
    status = GameStatus::PLAYING;

    notifyAll();
}

void GameState::nextPlayer() {
    switch (currentPlayer) {
        case Player::PLAYER1: currentPlayer = Player::PLAYER2; break;
        case Player::PLAYER2: currentPlayer = Player::PLAYER3; break;
        case Player::PLAYER3: currentPlayer = Player::PLAYER1; break;
        default: break;
    }
}

int GameState::evaluate() const {
    float score = 0.f;
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;
        if (p->getOwner() == currentPlayer)
            score += static_cast<float>(p->getValue());
        else
            score -= 0.5f * static_cast<float>(p->getValue());
    }
    return static_cast<int>(score);
}

void GameState::addObserver(IObserver* o) {
    observers.push_back(o);
}

void GameState::notifyAll() {
    for (auto o : observers)
        o->onStateChanged();
}