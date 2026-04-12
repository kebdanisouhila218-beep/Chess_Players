#include "GameState.hpp"
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

void GameState::applyMove(const Move& m) {
    Move applied = m;
    Piece* moving = board.getPiece(m.from);
    if (!moving) return;

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
                        board.removePiece(previous->to);
                        delete cap;
                    }
                }
            }
        }
    }

    if (capturedPiece) {
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
    board.movePiece(last.to, last.from);
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
    int score = 0;
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;
        if (p->getOwner() == Player::PLAYER1)
            score += p->getValue();
        else
            score -= p->getValue();
    }
    return score;
}

void GameState::addObserver(IObserver* o) {
    observers.push_back(o);
}

void GameState::notifyAll() {
    for (auto o : observers)
        o->onStateChanged();
}