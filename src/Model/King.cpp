#include "King.hpp"
#include "Board.hpp"
#include "BoardConstants.hpp"

std::vector<HexCell> King::getMoves(const Board& board) const {
    std::vector<HexCell> moves;

    // Deplacements normaux : 1 pas dans les 8 directions
    constexpr std::array<Board::Direction, 8> allDirs = {
        Board::Direction::NORTH,      Board::Direction::SOUTH,
        Board::Direction::EAST,       Board::Direction::WEST,
        Board::Direction::NORTH_EAST, Board::Direction::SOUTH_WEST,
        Board::Direction::SOUTH_EAST, Board::Direction::NORTH_WEST
    };
    for (Board::Direction dir : allDirs) {
        const auto target = board.step(pos, dir);
        if (!target.has_value()) continue;
        Piece* p = board.getPiece(*target);
        if (!p || p->getOwner() != owner)
            moves.push_back(*target);
    }

    // Roque : uniquement si le roi n'a pas bouge
    if (!hasMoved) {
        constexpr std::array<Board::Direction, 4> castleDirs = {
            Board::Direction::NORTH, Board::Direction::SOUTH,
            Board::Direction::EAST,  Board::Direction::WEST
        };
        for (Board::Direction dir : castleDirs) {
            const auto cells = board.ray(pos, dir);
            for (std::size_t i = 0; i < cells.size(); ++i) {
                Piece* p = board.getPiece(cells[i]);
                if (!p) continue;  // case vide — continuer a chercher

                // Premiere piece rencontree dans cette direction
                if (i >= 2 &&                           // place pour bouger de 2 cases
                    p->getType()  == PieceType::ROOK   &&
                    p->getOwner() == owner             &&
                    !p->getHasMoved())
                {
                    // Verifier que toutes les cases entre roi et tour sont vides
                    // (garanties vides par le scan ci-dessus, confirme via cellsBetween)
                    const auto between = board.cellsBetween(pos, cells[i]);
                    bool pathClear = true;
                    for (const HexCell& c : between) {
                        if (board.getPiece(c)) { pathClear = false; break; }
                    }
                    // TODO: verifier que le roi ne passe pas par une case attaquee (cells[0])
                    // TODO: verifier que la destination du roi (cells[1]) n'est pas attaquee
                    if (pathClear)
                        moves.push_back(cells[1]);  // roi se deplace de 2 cases vers la tour
                }
                break;  // arreter le scan dans cette direction (piece trouvee)
            }
        }
    }

    return moves;
}