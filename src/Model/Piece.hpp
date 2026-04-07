#pragma once
#include "HexCell.hpp"
#include <vector>

enum class Player {
    PLAYER1,
    PLAYER2,
    PLAYER3,
    NONE
};

enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

class Board;

class Piece {
public:
    Piece(PieceType type, Player owner, HexCell pos)
        : type(type), owner(owner), pos(pos) {}

    virtual ~Piece() = default;

    virtual std::vector<HexCell> getMoves(const Board& board) const = 0;
    virtual int getValue() const = 0;

    PieceType getType()   const { return type; }
    Player    getOwner()  const { return owner; }
    HexCell   getPos()    const { return pos; }
    void      setPos(HexCell p) { pos = p; }

    bool getHasMoved() const { return hasMoved; }
    void setHasMoved(bool v) { hasMoved = v; }

protected:
    PieceType type;
    Player    owner;
    HexCell   pos;
    bool      hasMoved = false;
};