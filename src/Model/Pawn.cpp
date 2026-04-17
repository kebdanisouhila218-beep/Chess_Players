#include "Pawn.hpp"
#include "Board.hpp"
#include "GameState.hpp"
#include <cmath>

#include <array>

namespace {
    Board::Direction pawnForward(const Board& board, const HexCell& pos) {
        const int sextant = board.getSextant(pos);
        return (sextant % 2 == 0) ? Board::Direction::SOUTH : Board::Direction::EAST;
    }

    std::array<Board::Direction, 2> pawnCaptures(const Board& board, const HexCell& pos) {
        const int sextant = board.getSextant(pos);
        if (sextant % 2 == 0) {
            return {Board::Direction::SOUTH_EAST, Board::Direction::SOUTH_WEST};
        }
        return {Board::Direction::NORTH_EAST, Board::Direction::SOUTH_EAST};
    }
}

std::vector<HexCell> Pawn::getMoves(const Board& board) const {
    return getMoves(board, nullptr);
}

std::vector<HexCell> Pawn::getCaptureSquares(const Board& board, const Move* lastMove) const {
    std::vector<HexCell> cells;
    for (Board::Direction captureDir : pawnCaptures(board, pos)) {
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
            const Board::Direction enemyForward = pawnForward(board, lastMove->from);
            std::optional<HexCell> mid = board.step(lastMove->from, enemyForward);
            std::optional<HexCell> end = mid.has_value() ? board.step(*mid, enemyForward) : std::nullopt;

            if (mid.has_value() && end.has_value() && *end == lastMove->to && board.getPiece(*mid) == nullptr) {
                for (Board::Direction captureDir : pawnCaptures(board, pos)) {
                    std::optional<HexCell> capture = board.step(pos, captureDir);
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

    const Board::Direction forward = pawnForward(board, pos);

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

    return moves;
}