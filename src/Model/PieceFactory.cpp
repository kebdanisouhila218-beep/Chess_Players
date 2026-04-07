#include "PieceFactory.hpp"

Piece* PieceFactory::create(PieceType type, Player owner, HexCell pos) {
    switch (type) {
        case PieceType::PAWN:   return new Pawn(owner, pos);
        case PieceType::KNIGHT: return new Knight(owner, pos);
        case PieceType::BISHOP: return new Bishop(owner, pos);
        case PieceType::ROOK:   return new Rook(owner, pos);
        case PieceType::QUEEN:  return new Queen(owner, pos);
        case PieceType::KING:   return new King(owner, pos);
        default: return nullptr;
    }
}

void PieceFactory::initBoard(Board& board, Player player) {
    auto place = [&](int x, int y, PieceType type) {
        HexCell cell{x, y};
        if (!board.isValid(cell) || board.getPiece(cell) != nullptr) {
            return;
        }
        board.setPiece(cell, create(type, player, cell));
    };

    if (player == Player::PLAYER1) {
        place(0, 0, PieceType::ROOK);
        place(1, 0, PieceType::KNIGHT);
        place(2, 0, PieceType::BISHOP);
        place(3, 0, PieceType::QUEEN);
        place(4, 0, PieceType::ROOK);
        place(5, 0, PieceType::PAWN);
        place(0, 1, PieceType::PAWN);
        place(1, 1, PieceType::PAWN);
        place(2, 1, PieceType::PAWN);
        place(3, 1, PieceType::PAWN);
        place(4, 1, PieceType::KNIGHT);
        place(5, 1, PieceType::PAWN);
        place(4, 2, PieceType::BISHOP);
        place(5, 2, PieceType::PAWN);
        place(4, 3, PieceType::KING);
        place(5, 3, PieceType::PAWN);
        return;
    }

    if (player == Player::PLAYER2) {
        place(0, 4, PieceType::ROOK);
        place(1, 4, PieceType::PAWN);
        place(8, 4, PieceType::ROOK);
        place(9, 4, PieceType::KNIGHT);
        place(10, 4, PieceType::BISHOP);
        place(11, 4, PieceType::QUEEN);
        place(0, 5, PieceType::KNIGHT);
        place(1, 5, PieceType::PAWN);
        place(8, 5, PieceType::PAWN);
        place(9, 5, PieceType::PAWN);
        place(10, 5, PieceType::PAWN);
        place(11, 5, PieceType::PAWN);
        place(0, 6, PieceType::BISHOP);
        place(1, 6, PieceType::PAWN);
        place(0, 7, PieceType::KING);
        place(1, 7, PieceType::PAWN);
        return;
    }

    if (player == Player::PLAYER3) {
        place(4, 8, PieceType::ROOK);
        place(5, 8, PieceType::KNIGHT);
        place(6, 8, PieceType::BISHOP);
        place(7, 8, PieceType::QUEEN);
        place(8, 8, PieceType::ROOK);
        place(9, 8, PieceType::PAWN);
        place(4, 9, PieceType::PAWN);
        place(5, 9, PieceType::PAWN);
        place(6, 9, PieceType::PAWN);
        place(7, 9, PieceType::PAWN);
        place(8, 9, PieceType::KNIGHT);
        place(9, 9, PieceType::PAWN);
        place(8, 10, PieceType::BISHOP);
        place(9, 10, PieceType::PAWN);
        place(8, 11, PieceType::KING);
        place(9, 11, PieceType::PAWN);
    }
}