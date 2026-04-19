#pragma once
#include "Board.hpp"
#include "Piece.hpp"

class PawnRefactored : public Piece {
public:
    PawnRefactored(Player owner, HexCell pos);
    std::vector<HexCell> getMoves(const Board& board) const override;
    std::vector<HexCell> getCaptureSquares(const Board& board) const override;

private:
    struct MoveResult {
        std::vector<HexCell> moves;
        std::vector<HexCell> captures;
    };
    
    MoveResult calculateAllMoves(const Board& board) const;
    Board::Direction getForwardDirection(const Board& board) const;
    std::array<Board::Direction, 2> getCaptureDirections(const Board& board) const;
    bool isAtSeam(const Board& board) const;
    std::optional<HexCell> getSeamTransition(const Board& board) const;
};
