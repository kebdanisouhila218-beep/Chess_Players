#include "Pawn.hpp"
#include "Board.hpp"
#include "GameState.hpp"
#include <cmath>

#include <array>

namespace {
    Board::Direction pawnForward(Player owner) {
        if (owner == Player::PLAYER1) return Board::Direction::SOUTH;
        if (owner == Player::PLAYER2) return Board::Direction::EAST;
        return Board::Direction::NORTH;
    }

    std::array<Board::Direction, 2> pawnCaptures(Player owner) {
        if (owner == Player::PLAYER1) {
            return {Board::Direction::SOUTH_EAST, Board::Direction::SOUTH_WEST};
        }
        if (owner == Player::PLAYER2) {
            return {Board::Direction::NORTH_EAST, Board::Direction::SOUTH_EAST};
        }
        return {Board::Direction::NORTH_EAST, Board::Direction::NORTH_WEST};
    }
}

std::vector<HexCell> Pawn::getMoves(const Board& board) const {
    return getMoves(board, nullptr);
}

std::vector<HexCell> Pawn::getMoves(const Board& board, const Move* lastMove) const {
    std::vector<HexCell> moves;
    const Board::Direction forward = pawnForward(owner);

    if (std::optional<HexCell> transition = board.getPawnTransition(pos, owner)) {
        if (board.isValid(*transition) && board.getPiece(*transition) == nullptr) {
            moves.push_back(*transition);
        }
        return moves;
    }

    std::optional<HexCell> front = board.step(pos, forward);
    if (front.has_value() && board.getPiece(*front) == nullptr) {
        moves.push_back(*front);
        if (!getHasMoved()) {
            std::optional<HexCell> front2 = board.step(*front, forward);
            if (front2.has_value() && board.getPiece(*front2) == nullptr) {
                moves.push_back(*front2);
            }
        }
    }

    for (Board::Direction captureDir : pawnCaptures(owner)) {
        std::optional<HexCell> capture = board.step(pos, captureDir);
        if (!capture.has_value()) {
            continue;
        }
        Piece* target = board.getPiece(*capture);
        if (target && target->getOwner() != owner) {
            moves.push_back(*capture);
        }
    }

    if (lastMove && lastMove->player != owner) {
        Piece* movedPiece = board.getPiece(lastMove->to);
        if (movedPiece && movedPiece->getType() == PieceType::PAWN) {
            const int dx = lastMove->to.q - lastMove->from.q;
            const int dy = lastMove->to.r - lastMove->from.r;
            if (std::abs(dx) + std::abs(dy) == 2) {
                for (Board::Direction captureDir : pawnCaptures(owner)) {
                    std::optional<HexCell> capture = board.step(pos, captureDir);
                    if (capture.has_value() && *capture == lastMove->to) {
                        std::optional<HexCell> epTarget = board.step(lastMove->from, forward);
                        if (epTarget.has_value() && board.getPiece(*epTarget) == nullptr) {
                            moves.push_back(*epTarget);
                        }
                    }
                }
            }
        }
    }

    return moves;
}