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

// LIMITATION CONNUE : undoMove() est actuellement incomplet.
// Il restaure uniquement la position de la piece deplacee,
// mais ne restaure PAS :
//   - la piece capturee (deja delete, perdue definitivement)
//   - l'etat hasMoved de la piece deplacee
//   - la piece promue (le pion original est detruit a la promotion)
//   - la tour deplacee lors d'un roque
//   - currentPlayer
//   - status
//
// Pour corriger proprement, il faudra enrichir struct Move avec :
//   - capturedType / capturedOwner / capturedCell
//   - previousHasMoved
//   - originalPawnType en cas de promotion
//   - rookFrom / rookTo deja presents mais non restaures
//
// undoMove() n'est pas utilise en jeu normal actuellement.
// A traiter dans une etape dediee si on implemente un moteur IA
// ou une fonction "annuler le coup".
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