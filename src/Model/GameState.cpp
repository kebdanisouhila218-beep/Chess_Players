#include "GameState.hpp"

GameState::GameState()
    : currentPlayer(Player::PLAYER1)
    , status(GameStatus::PLAYING)
{}

void GameState::applyMove(const Move& m) {
    board.movePiece(m.from, m.to);
    moveHistory.push_back(m);
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