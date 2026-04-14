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
    // getValue() reste separe de getMoves() pour distinguer
    // la logique de deplacement de la logique d'evaluation.
    // Une piece peut avoir les memes coups potentiels, mais une valeur
    // strategique differente pour l'IA, le score ou de futures simulations.
    virtual int getValue() const = 0;

    PieceType getType()   const { return type; }
    Player    getOwner()  const { return owner; }
    HexCell   getPos()    const { return pos; }
    void      setPos(HexCell p) { pos = p; }

    bool isEnemy(const Piece* other) const {
        return other != nullptr && other->getOwner() != owner;
    }

    bool getHasMoved() const { return hasMoved; }
    void setHasMoved(bool v) { hasMoved = v; }

protected:
    PieceType type;
    Player    owner;
    HexCell   pos;
    bool      hasMoved = false;
};