#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "../Model/Board.hpp"

// ---------------------------------------------------------------------------
// Temporary debug helper — remove call from main.cpp when done.
// Zero changes to any model/view/controller code.
// ---------------------------------------------------------------------------

namespace debug {

inline std::string pieceStr(const Piece* p) {
    if (!p) return "empty";

    const char* type = "?";
    switch (p->getType()) {
        case PieceType::PAWN:   type = "PAWN";   break;
        case PieceType::KNIGHT: type = "KNIGHT"; break;
        case PieceType::BISHOP: type = "BISHOP"; break;
        case PieceType::ROOK:   type = "ROOK";   break;
        case PieceType::QUEEN:  type = "QUEEN";  break;
        case PieceType::KING:   type = "KING";   break;
        default: break;
    }

    const char* owner = "??";
    switch (p->getOwner()) {
        case Player::PLAYER1: owner = "P1"; break;
        case Player::PLAYER2: owner = "P2"; break;
        case Player::PLAYER3: owner = "P3"; break;
        default: break;
    }

    return std::string(type) + "/" + owner;
}

inline void printBoardLayout(const Board& board) {
    // --- Section 1: cell list sorted by id ---
    std::vector<HexCell> cells = board.allValidCells();
    std::sort(cells.begin(), cells.end(),
              [&](const HexCell& a, const HexCell& b) {
                  return board.getId(a) < board.getId(b);
              });

    std::cout << "=== BOARD LAYOUT (" << cells.size() << " cells) ===\n";
    for (const HexCell& c : cells) {
        const int id  = board.getId(c);
        const int sxt = board.getSextant(c);
        const Piece* p = board.getPiece(c);
        std::cout << "id=(" << std::setw(2) << id << ")"
                  << " xy=(" << c.q << "," << c.r << ")"
                  << " sextant=" << sxt
                  << " piece=" << pieceStr(p)
                  << "\n";
    }

    // --- Section 2: 12x12 visual grid ---
    // Each cell is 4 chars: "  . " / " __ " / " XX "
    std::cout << "\n=== XY MAP ===\n";
    std::cout << "      ";
    for (int x = 0; x < Board::BOARD_SIZE; ++x)
        std::cout << " x" << std::left << std::setw(2) << x;
    std::cout << std::right << "\n";

    for (int y = 0; y < Board::BOARD_SIZE; ++y) {
        std::cout << "y=" << std::setw(2) << y << "  ";
        for (int x = 0; x < Board::BOARD_SIZE; ++x) {
            const HexCell c{x, y};
            const int id = board.getId(c);
            if (id < 0) {
                std::cout << "  . ";
            } else {
                const Piece* p = board.getPiece(c);
                if (p)
                    std::cout << " " << std::setfill('0') << std::setw(2) << id
                              << std::setfill(' ') << " ";
                else
                    std::cout << " __ ";
            }
        }
        std::cout << "\n";
    }

    std::cout << std::flush;
}

} // namespace debug
