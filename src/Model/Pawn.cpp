#include "Pawn.hpp"
#include "Board.hpp"
#include "GameState.hpp"
#include <cmath>

namespace {
    HexCell pawnForward(Player owner) {
        if (owner == Player::PLAYER1) return {0, 1};
        if (owner == Player::PLAYER2) return {1, 0};
        return {0, -1};
    }
}

std::vector<HexCell> Pawn::getMoves(const Board& board) const {
    return getMoves(board, nullptr);
}

std::vector<HexCell> Pawn::getMoves(const Board& board, const Move* lastMove) const {
    std::vector<HexCell> moves;

    if (std::optional<HexCell> transition = board.getPawnTransition(pos, owner)) {
        if (board.isValid(*transition) && board.getPiece(*transition) == nullptr) {
            moves.push_back(*transition);
        }
        return moves;
    }

    const HexCell forward = pawnForward(owner);
    HexCell front{pos.q + forward.q, pos.r + forward.r};
    if (board.isValid(front) && board.getPiece(front) == nullptr) {
        moves.push_back(front);
        if (!getHasMoved()) {
            HexCell front2{front.q + forward.q, front.r + forward.r};
            if (board.isValid(front2) && board.getPiece(front2) == nullptr) {
                moves.push_back(front2);
            }
        }
    }

    const HexCell cap1{pos.q + forward.q + forward.r, pos.r + forward.r - forward.q};
    const HexCell cap2{pos.q + forward.q - forward.r, pos.r + forward.r + forward.q};
    for (const HexCell& capture : {cap1, cap2}) {
        if (!board.isValid(capture)) {
            continue;
        }
        Piece* target = board.getPiece(capture);
        if (target && target->getOwner() != owner) {
            moves.push_back(capture);
        }
    }

    if (lastMove && lastMove->player != owner) {
        Piece* movedPiece = board.getPiece(lastMove->to);
        if (movedPiece && movedPiece->getType() == PieceType::PAWN) {
            const int dx = lastMove->to.q - lastMove->from.q;
            const int dy = lastMove->to.r - lastMove->from.r;
            if ((dx == 2 * forward.q && dy == 2 * forward.r) ||
                (std::abs(dx) + std::abs(dy) == 2 && board.getZoneOwner(lastMove->to) != owner)) {
                for (const HexCell& capture : {cap1, cap2}) {
                    if (capture == lastMove->to) {
                        HexCell epTarget{lastMove->from.q + forward.q, lastMove->from.r + forward.r};
                        if (board.isValid(epTarget) && board.getPiece(epTarget) == nullptr) {
                            moves.push_back(epTarget);
                        }
                    }
                }
            }
        }
    }

    return moves;
}