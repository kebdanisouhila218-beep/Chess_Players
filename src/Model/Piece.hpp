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
    virtual ~Piece() = default; //detruire une piece -pion- virtual pour ne pas avoir de fuite de memoire
 
    virtual std::vector<HexCell> getMoves(const Board& board) const = 0;
    // getValue() reste separe de getMoves() pour distinguer
    // la logique de deplacement de la logique d'evaluation.
    // Une piece peut avoir les memes coups potentiels, mais une valeur
    // strategique differente pour l'IA, le score ou de futures simulations.
    virtual int getValue() const = 0;
//Encapsulation
    PieceType getType()   const { return type; }
    Player    getOwner()  const { return owner; }
    HexCell   getPos()    const { return pos; }
    void      setPos(HexCell p) { pos = p; }
    void      setOwner(Player p) { owner = p; }

    bool isEnemy(const Piece* other) const {
       // other c est la case qui contient la piece a verifier
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