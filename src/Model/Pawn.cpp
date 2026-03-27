#include "Pawn.hpp"
#include "Board.hpp"

std::vector<HexCell> Pawn::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    // Direction d'avance selon le joueur
    std::vector<HexCell> forward;
    if (owner == Player::PLAYER1) forward = {{pos.q, pos.r-1}, {pos.q+1, pos.r-1}};
    if (owner == Player::PLAYER2) forward = {{pos.q-1, pos.r+1}, {pos.q-1, pos.r}};
    if (owner == Player::PLAYER3) forward = {{pos.q+1, pos.r}, {pos.q, pos.r+1}};

    // Avance d'une case si vide
    HexCell front = forward[0];
    if (board.isValid(front) && board.getPiece(front) == nullptr)
        moves.push_back(front);

    // Capture en diagonale
    HexCell diag = forward[1];
    Piece* target = board.getPiece(diag);
    if (board.isValid(diag) && target != nullptr && target->getOwner() != owner)
        moves.push_back(diag);

    return moves;
}