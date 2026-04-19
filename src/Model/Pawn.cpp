#include "Pawn.hpp"
#include "Board.hpp"
#include "GameState.hpp"
#include <cmath>

#include <array>

namespace {
    Board::Direction pawnForward(const Board& board, Player owner, const HexCell& pos) {
        const int sextant = board.getSextant(pos);
        switch (owner) {
            case Player::PLAYER1:
                if (sextant == 0) return Board::Direction::SOUTH;
                if (sextant == 1) return Board::Direction::SOUTH;
                if (sextant == 2) return Board::Direction::SOUTH;
                if (sextant == 3) return Board::Direction::SOUTH;
                if (sextant == 4) return Board::Direction::SOUTH;
                if (sextant == 5) return Board::Direction::EAST;
                break;
            case Player::PLAYER2:
                if (sextant == 0) return Board::Direction::SOUTH;
                if (sextant == 1) return Board::Direction::EAST;
                if (sextant == 2) return Board::Direction::SOUTH;
                if (sextant == 3) return Board::Direction::WEST;
                if (sextant == 4) return Board::Direction::NORTH;
                if (sextant == 5) return Board::Direction::NORTH;
                break;
            case Player::PLAYER3:
                if (sextant == 0) return Board::Direction::NORTH;
                if (sextant == 1) return Board::Direction::NORTH;
                if (sextant == 2) return Board::Direction::NORTH;
                if (sextant == 3) return Board::Direction::EAST;
                if (sextant == 4) return Board::Direction::SOUTH;
                if (sextant == 5) return Board::Direction::WEST;
                break;
            default:
                break;
        }
        return Board::Direction::SOUTH;
    }

    bool crossesSeam(const Board& board, const HexCell& from, const std::optional<HexCell>& to) {
        if (!to.has_value()) {
            return false;
        }
        if (board.getZoneOwner(from) != board.getZoneOwner(*to)) {
            return true;
        }
        return from.manhattanLikeDistance(*to) > 2;
    }

    std::array<Board::Direction, 2> pawnCaptures(const Board& board, Player owner, const HexCell& pos, bool crossingSeam) {
        if (crossingSeam) {
            return {Board::Direction::NORTH_EAST, Board::Direction::NORTH_WEST};
        }

        const Board::Direction forward = pawnForward(board, owner, pos);
        switch (forward) {
            case Board::Direction::EAST:
                return {Board::Direction::NORTH_EAST, Board::Direction::SOUTH_EAST};
            case Board::Direction::WEST:
                return {Board::Direction::NORTH_WEST, Board::Direction::SOUTH_WEST};
            case Board::Direction::NORTH:
                return {Board::Direction::NORTH_EAST, Board::Direction::NORTH_WEST};
            case Board::Direction::SOUTH:
            default:
                return {Board::Direction::SOUTH_EAST, Board::Direction::SOUTH_WEST};
        }
    }
}

std::vector<HexCell> Pawn::getMoves(const Board& board) const {
    return getMoves(board, nullptr);
}

std::vector<HexCell> Pawn::getCaptureSquares(const Board& board, const Move* lastMove) const {
    std::vector<HexCell> cells;
    const Board::Direction forward = pawnForward(board, owner, pos);
    const std::optional<HexCell> front = board.step(pos, forward);
    const std::optional<HexCell> transition = !front.has_value() ? board.getPawnTransition(pos, owner) : std::nullopt;
    const bool directSeamForward = crossesSeam(board, pos, front);

    const bool crossingSeam = transition.has_value() || directSeamForward;
    const HexCell captureBase = transition.has_value() ? *transition : pos;

    // If transition is occupied by enemy, it should be a valid capture target
    if (transition.has_value()) {
        Piece* transitionPiece = board.getPiece(*transition);
        if (transitionPiece && transitionPiece->getOwner() != owner) {
            cells.push_back(*transition);
        }
    }

    // If direct seam forward is occupied by enemy, it should be a valid capture target
    if (directSeamForward && front.has_value()) {
        Piece* frontPiece = board.getPiece(*front);
        if (frontPiece && frontPiece->getOwner() != owner) {
            cells.push_back(*front);
        }
    }

    // Only proceed with diagonal captures from original position if no transition
    // If there's a transition, don't allow diagonal captures from the transition position
    if (transition.has_value() || directSeamForward) {
        // Only diagonal captures allowed are direct captures on transition/forward squares (handled above)
        return cells;
    }

    for (Board::Direction captureDir : pawnCaptures(board, owner, pos, false)) {
        std::optional<HexCell> capture = board.step(pos, captureDir);
        if (capture.has_value() && board.isValid(*capture)) {
            Piece* target = board.getPiece(*capture);
            if (target && target->getOwner() != owner) {
                cells.push_back(*capture);
            }
        }
    }

    if (lastMove && lastMove->player != owner) {
        Piece* movedPiece = board.getPiece(lastMove->to);
        if (movedPiece && movedPiece->getType() == PieceType::PAWN) {
            const Board::Direction enemyForward = pawnForward(board, lastMove->player, lastMove->from);
            std::optional<HexCell> mid = board.step(lastMove->from, enemyForward);
            std::optional<HexCell> end = mid.has_value() ? board.step(*mid, enemyForward) : std::nullopt;

            if (mid.has_value() && end.has_value() && *end == lastMove->to && board.getPiece(*mid) == nullptr) {
                for (Board::Direction captureDir : pawnCaptures(board, owner, captureBase, crossingSeam)) {
                    std::optional<HexCell> capture = board.step(captureBase, captureDir);
                    if (capture.has_value() && *capture == *mid) {
                        cells.push_back(*capture);
                    }
                }
            }
        }
    }

    return cells;
}

std::vector<HexCell> Pawn::getMoves(const Board& board, const Move* lastMove) const {
    std::vector<HexCell> moves;

    const Board::Direction forward = pawnForward(board, owner, pos);

    std::optional<HexCell> front = board.step(pos, forward);

    if (!front.has_value()) {
        if (std::optional<HexCell> transition = board.getPawnTransition(pos, owner)) {
            if (board.isValid(*transition) && board.getPiece(*transition) == nullptr) {
                moves.push_back(*transition);
            }
        }
        return moves;
    }

    if (front.has_value() && board.getPiece(*front) == nullptr) {
        moves.push_back(*front);
        if (!getHasMoved()) {
            std::optional<HexCell> front2 = board.step(*front, forward);
            if (front2.has_value() && board.getPiece(*front2) == nullptr) {
                moves.push_back(*front2);
            }
        }
    }

    return moves;
}