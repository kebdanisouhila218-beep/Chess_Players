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
    // Positions initiales selon le joueur
    // Joueur 1 : rangée du bas
    if (player == Player::PLAYER1) {
        board.setPiece({-4, 4}, create(PieceType::ROOK,   player, {-4, 4}));
        board.setPiece({-3, 4}, create(PieceType::KNIGHT, player, {-3, 4}));
        board.setPiece({-2, 4}, create(PieceType::BISHOP, player, {-2, 4}));
        board.setPiece({-1, 4}, create(PieceType::QUEEN,  player, {-1, 4}));
        board.setPiece({ 0, 4}, create(PieceType::KING,   player, { 0, 4}));
        board.setPiece({ 1, 4}, create(PieceType::BISHOP, player, { 1, 4}));
        board.setPiece({ 2, 4}, create(PieceType::KNIGHT, player, { 2, 4}));
        board.setPiece({ 3, 4}, create(PieceType::ROOK,   player, { 3, 4}));
        // Pions
        for (int q = -4; q <= 4; q++)
            board.setPiece({q, 3}, create(PieceType::PAWN, player, {q, 3}));
    }

    // Joueur 2 : rangée droite
    if (player == Player::PLAYER2) {
        board.setPiece({ 4,-4}, create(PieceType::ROOK,   player, { 4,-4}));
        board.setPiece({ 4,-3}, create(PieceType::KNIGHT, player, { 4,-3}));
        board.setPiece({ 4,-2}, create(PieceType::BISHOP, player, { 4,-2}));
        board.setPiece({ 4,-1}, create(PieceType::QUEEN,  player, { 4,-1}));
        board.setPiece({ 4, 0}, create(PieceType::KING,   player, { 4, 0}));
        board.setPiece({ 4, 1}, create(PieceType::BISHOP, player, { 4, 1}));
        board.setPiece({ 4, 2}, create(PieceType::KNIGHT, player, { 4, 2}));
        board.setPiece({ 4, 3}, create(PieceType::ROOK,   player, { 4, 3}));
        // Pions
        for (int r = -4; r <= 4; r++)
            board.setPiece({3, r}, create(PieceType::PAWN, player, {3, r}));
    }

    // Joueur 3 : rangée gauche
    if (player == Player::PLAYER3) {
        board.setPiece({-4, 0}, create(PieceType::ROOK,   player, {-4, 0}));
        board.setPiece({-4, 1}, create(PieceType::KNIGHT, player, {-4, 1}));
        board.setPiece({-4, 2}, create(PieceType::BISHOP, player, {-4, 2}));
        board.setPiece({-4, 3}, create(PieceType::QUEEN,  player, {-4, 3}));
        board.setPiece({-4, 4}, create(PieceType::KING,   player, {-4, 4}));
        board.setPiece({-4,-1}, create(PieceType::BISHOP, player, {-4,-1}));
        board.setPiece({-4,-2}, create(PieceType::KNIGHT, player, {-4,-2}));
        board.setPiece({-4,-3}, create(PieceType::ROOK,   player, {-4,-3}));
        // Pions
        for (int r = -3; r <= 3; r++)
            board.setPiece({-3, r}, create(PieceType::PAWN, player, {-3, r}));
    }
}