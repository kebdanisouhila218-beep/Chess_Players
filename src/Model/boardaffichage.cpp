#include "Board.hpp"
#include "Piece.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

void printBoardAsMatrix(const Board& board) {
    auto pieceCode = [](const Piece* piece) -> std::string {
        if (!piece) {
            return "";
        }

        std::string playerPart;
        switch (piece->getOwner()) {
            case Player::PLAYER1: playerPart = "P1"; break;
            case Player::PLAYER2: playerPart = "P2"; break;
            case Player::PLAYER3: playerPart = "P3"; break;
            default: playerPart = "P?"; break;
        }

        char typePart = '?';
        switch (piece->getType()) {
            case PieceType::KING:   typePart = 'K'; break;
            case PieceType::QUEEN:  typePart = 'Q'; break;
            case PieceType::ROOK:   typePart = 'R'; break;
            case PieceType::BISHOP: typePart = 'B'; break;
            case PieceType::KNIGHT: typePart = 'N'; break;
            case PieceType::PAWN:   typePart = 'p'; break;
            default:                typePart = '?'; break;
        }

        return playerPart + typePart;
    };

    std::cout << "      ";
    for (int x = 0; x < 12; ++x) {
        std::cout << std::setw(6) << x;
    }
    std::cout << '\n';

    for (int y = 0; y < 12; ++y) {
        std::cout << std::setw(4) << y << "  ";

        for (int x = 0; x < 12; ++x) {
            HexCell cell{x, y};

            if (!board.isValid(cell)) {
                std::cout << std::setw(6) << ".";
                continue;
            }

            Piece* piece = board.getPiece(cell);
            if (piece) {
                std::cout << std::setw(6) << pieceCode(piece);
            } else {
                std::string coord = "(" + std::to_string(x) + "," + std::to_string(y) + ")";
                std::cout << std::setw(6) << coord;
            }
        }

        std::cout << '\n';
    }
}

void printBoardVisualLayout(const Board& board) {
    struct VisualCell {
        HexCell cell;
        ScreenCoord screen;
    };

    std::vector<VisualCell> cells;
    cells.reserve(Board::CELL_COUNT);

    for (const HexCell& cell : board.allValidCells()) {
        cells.push_back({cell, board.getScreenPosition(cell)});
    }

    std::sort(cells.begin(), cells.end(), [](const VisualCell& a, const VisualCell& b) {
        const float epsilon = 0.01f;
        if (std::fabs(a.screen.y - b.screen.y) > epsilon) {
            return a.screen.y < b.screen.y;
        }
        return a.screen.x < b.screen.x;
    });

    std::vector<std::vector<VisualCell>> rows;
    const float rowTolerance = 1.0f;

    for (const VisualCell& visualCell : cells) {
        if (rows.empty() || std::fabs(rows.back().front().screen.y - visualCell.screen.y) > rowTolerance) {
            rows.push_back({visualCell});
        } else {
            rows.back().push_back(visualCell);
        }
    }

    float minX = cells.empty() ? 0.f : cells.front().screen.x;
    for (const VisualCell& visualCell : cells) {
        if (visualCell.screen.x < minX) {
            minX = visualCell.screen.x;
        }
    }

    const float indentStep = 17.0f;

    for (const auto& row : rows) {
        const float rowMinX = row.front().screen.x;
        int indentCount = static_cast<int>(std::round((rowMinX - minX) / indentStep));
        if (indentCount < 0) {
            indentCount = 0;
        }

        for (int i = 0; i < indentCount; ++i) {
            std::cout << ' ';
        }

        for (const VisualCell& visualCell : row) {
            std::cout << '(' << visualCell.cell.q << ',' << visualCell.cell.r << ") ";
        }
        std::cout << '\n';
    }
}