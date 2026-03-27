#include "Board.hpp"

const int Board::RADIUS;
Board::Board() {
    // Génère les 91 cases du plateau Glinski (rayon 5)
    for (int q = -RADIUS; q <= RADIUS; q++) {
        int r1 = std::max(-RADIUS, -q - RADIUS);
        int r2 = std::min( RADIUS, -q + RADIUS);
        for (int r = r1; r <= r2; r++) {
            cells[{q, r}] = nullptr;
        }
    }
}

bool Board::isValid(const HexCell& c) const {
    return cells.count(c) > 0;
}

Piece* Board::getPiece(const HexCell& c) const {
    auto it = cells.find(c);
    if (it != cells.end()) return it->second;
    return nullptr;
}

void Board::setPiece(const HexCell& c, Piece* p) {
    if (isValid(c)) cells[c] = p;
}

void Board::removePiece(const HexCell& c) {
    if (isValid(c)) cells[c] = nullptr;
}

void Board::movePiece(const HexCell& from, const HexCell& to) {
    Piece* p = getPiece(from);
    if (p) {
        setPiece(to, p);
        removePiece(from);
        p->setPos(to);
    }
}

std::vector<HexCell> Board::allValidCells() const {
    std::vector<HexCell> result;
    for (auto& [cell, piece] : cells)
        result.push_back(cell);
    return result;
}