#include "Model/Board.hpp"
#include "Model/GameState.hpp"
#include "Model/Pawn.hpp"
#include "Model/Knight.hpp"
#include "Model/Bishop.hpp"
#include "Model/Rook.hpp"
#include "Model/Queen.hpp"
#include "Model/King.hpp"
#include "Model/PieceFactory.hpp"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace {
    using PieceStore = std::vector<std::unique_ptr<Piece>>;

    struct PieceSnapshot {
        bool present = false;
        PieceType type = PieceType::PAWN;
        Player owner = Player::NONE;
        bool hasMoved = false;
    };

    std::string playerText(Player player) {
        switch (player) {
            case Player::PLAYER1: return "Blanc";
            case Player::PLAYER2: return "Bleu";
            case Player::PLAYER3: return "Rouge";
            default: return "None";
        }
    }

    std::string pieceText(PieceType type) {
        switch (type) {
            case PieceType::PAWN: return "Pawn";
            case PieceType::KNIGHT: return "Knight";
            case PieceType::BISHOP: return "Bishop";
            case PieceType::ROOK: return "Rook";
            case PieceType::QUEEN: return "Queen";
            case PieceType::KING: return "King";
            default: return "?";
        }
    }

    std::string cellText(const HexCell& cell) {
        std::ostringstream out;
        out << '(' << cell.q << ',' << cell.r << ')';
        return out.str();
    }

    std::string moveListText(std::vector<HexCell> moves) {
        std::sort(moves.begin(), moves.end(), [](const HexCell& a, const HexCell& b) {
            return a.q < b.q || (a.q == b.q && a.r < b.r);
        });

        std::ostringstream out;
        for (std::size_t i = 0; i < moves.size(); ++i) {
            if (i > 0) {
                out << ' ';
            }
            out << cellText(moves[i]);
        }
        return out.str();
    }

    void clearBoard(Board& board) {
        for (const HexCell& cell : board.allValidCells()) {
            board.removePiece(cell);
        }
    }

    template <typename T>
    T* addPiece(Board& board, PieceStore& pieces, Player owner, HexCell cell) {
        auto piece = std::make_unique<T>(owner, cell);
        T* raw = piece.get();
        board.setPiece(cell, raw);
        pieces.push_back(std::move(piece));
        return raw;
    }

    void initStandardPosition(GameState& state) {
        PieceFactory factory;
        factory.initBoard(state.getBoard(), Player::PLAYER1);
        factory.initBoard(state.getBoard(), Player::PLAYER2);
        factory.initBoard(state.getBoard(), Player::PLAYER3);
    }

    bool contains(const std::vector<HexCell>& moves, const HexCell& target) {
        return std::find(moves.begin(), moves.end(), target) != moves.end();
    }

    std::vector<std::pair<HexCell, PieceSnapshot>> snapshotBoard(const Board& board) {
        std::vector<std::pair<HexCell, PieceSnapshot>> snapshot;
        for (const HexCell& cell : board.allValidCells()) {
            PieceSnapshot state;
            if (Piece* piece = board.getPiece(cell)) {
                state.present = true;
                state.type = piece->getType();
                state.owner = piece->getOwner();
                state.hasMoved = piece->getHasMoved();
            }
            snapshot.push_back({cell, state});
        }
        return snapshot;
    }

    bool boardMatchesSnapshot(const Board& board,
                              const std::vector<std::pair<HexCell, PieceSnapshot>>& snapshot,
                              std::string& mismatch) {
        for (const auto& [cell, expected] : snapshot) {
            Piece* piece = board.getPiece(cell);
            const bool present = piece != nullptr;
            if (present != expected.present) {
                mismatch = cellText(cell) + " presence mismatch";
                return false;
            }
            if (!present) {
                continue;
            }
            if (piece->getType() != expected.type) {
                mismatch = cellText(cell) + " type mismatch";
                return false;
            }
            if (piece->getOwner() != expected.owner) {
                mismatch = cellText(cell) + " owner mismatch";
                return false;
            }
            if (piece->getHasMoved() != expected.hasMoved) {
                mismatch = cellText(cell) + " hasMoved mismatch";
                return false;
            }
        }
        return true;
    }

    struct TestCase {
        std::string name;
        std::function<bool()> run;
    };

    bool reportResult(const std::string& name, bool ok, const std::string& details) {
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << '\n';
        if (!details.empty()) {
            std::cout << "       " << details << '\n';
        }
        return ok;
    }

    bool testBoardStepSeams() {
        Board board;
        bool ok1 = false;
        bool ok2 = false;
        bool ok3 = false;

        if (std::optional<HexCell> step = board.step({7, 0}, Board::Direction::SOUTH)) {
            ok1 = (*step == HexCell{4, 11});
        }
        if (std::optional<HexCell> step = board.step({0, 3}, Board::Direction::SOUTH)) {
            ok2 = (*step == HexCell{3, 4});
        }
        if (std::optional<HexCell> step = board.step({8, 7}, Board::Direction::SOUTH)) {
            ok3 = (*step == HexCell{11, 8});
        }

        std::ostringstream details;
        details << "(7,0)->S " << (ok1 ? "OK" : "BAD") << ", "
                << "(0,3)->S " << (ok2 ? "OK" : "BAD") << ", "
                << "(8,7)->S " << (ok3 ? "OK" : "BAD");
        return reportResult("Board seam transitions", ok1 && ok2 && ok3, details.str());
    }

    bool testBoardDiagonalSeamTransitions() {
        Board board;
        bool ok = true;
        std::vector<std::string> checks;

        const std::array<std::tuple<HexCell, Board::Direction, HexCell>, 6> cases = {{
            {{7, 1}, Board::Direction::SOUTH_WEST, {4, 11}},
            {{7, 1}, Board::Direction::SOUTH_EAST, {6, 11}},
            {{1, 3}, Board::Direction::SOUTH_WEST, {3, 4}},
            {{1, 3}, Board::Direction::SOUTH_EAST, {3, 6}},
            {{9, 7}, Board::Direction::SOUTH_WEST, {11, 8}},
            {{9, 7}, Board::Direction::SOUTH_EAST, {11, 10}}
        }};

        for (const auto& [from, dir, expected] : cases) {
            const std::optional<HexCell> step = board.step(from, dir);
            const bool localOk = step.has_value() && *step == expected;
            ok = ok && localOk;
            std::ostringstream out;
            out << cellText(from) << " -> " << (localOk ? cellText(expected) : "BAD");
            checks.push_back(out.str());
        }

        std::ostringstream details;
        for (std::size_t i = 0; i < checks.size(); ++i) {
            if (i > 0) details << ", ";
            details << checks[i];
        }
        return reportResult("Board diagonal seam transitions", ok, details.str());
    }

    bool testBoardValidityMap() {
        Board board;
        int validCount = 0;
        int invalidCount = 0;
        bool matchesIntervals = true;

        for (int y = 0; y < Board::BOARD_SIZE; ++y) {
            for (int x = 0; x < Board::BOARD_SIZE; ++x) {
                const bool expected =
                    (x >= 0 && x < 4 && y >= 0 && y < 4) ||
                    (x >= 0 && x < 4 && y >= 4 && y < 8) ||
                    (x >= 8 && x < 12 && y >= 4 && y < 8) ||
                    (x >= 8 && x < 12 && y >= 8 && y < 12) ||
                    (x >= 4 && x < 8 && y >= 8 && y < 12) ||
                    (x >= 4 && x < 8 && y >= 0 && y < 4);
                const bool actual = board.isValid({x, y});
                if (actual) {
                    ++validCount;
                } else {
                    ++invalidCount;
                }
                if (actual != expected) {
                    matchesIntervals = false;
                }
            }
        }

        std::ostringstream details;
        details << "valid=" << validCount << " invalid=" << invalidCount
                << " cell_count=" << board.allValidCells().size();
        return reportResult("Board validity map matches 96-cell layout",
            validCount == 96 && invalidCount == 48 && matchesIntervals && board.allValidCells().size() == 96,
            details.str());
    }

    bool testBoardNeighborSymmetry() {
        Board board;
        bool ok = true;
        int checkedSteps = 0;
        int checkedSeams = 0;
        std::vector<std::string> mismatches;

        const std::array<Board::Direction, 8> dirs = {
            Board::Direction::NORTH,
            Board::Direction::SOUTH,
            Board::Direction::EAST,
            Board::Direction::WEST,
            Board::Direction::NORTH_EAST,
            Board::Direction::SOUTH_WEST,
            Board::Direction::SOUTH_EAST,
            Board::Direction::NORTH_WEST
        };

        for (const HexCell& cell : board.allValidCells()) {
            for (Board::Direction dir : dirs) {
                std::optional<HexCell> next = board.step(cell, dir);
                if (!next.has_value()) {
                    continue;
                }
                ++checkedSteps;
                if (!board.isValid(*next)) {
                    ok = false;
                    if (mismatches.size() < 8) {
                        mismatches.push_back(cellText(cell) + " -> invalid target " + cellText(*next));
                    }
                }
            }
        }

        for (int k = 0; k < 4; ++k) {
            const std::array<std::pair<HexCell, HexCell>, 3> seams = {{
                {HexCell{7, k}, HexCell{4 + k, 11}},
                {HexCell{k, 3}, HexCell{3, 4 + k}},
                {HexCell{8 + k, 7}, HexCell{11, 8 + k}}
            }};

            for (const auto& [source, target] : seams) {
                ++checkedSeams;
                std::optional<HexCell> south = board.step(source, Board::Direction::SOUTH);
                std::optional<HexCell> north = board.step(target, Board::Direction::NORTH);
                if (!south.has_value() || !(*south == target) || !north.has_value() || !(*north == source)) {
                    ok = false;
                    if (mismatches.size() < 8) {
                        std::ostringstream entry;
                        entry << cellText(source) << " <-> " << cellText(target) << " seam broken";
                        mismatches.push_back(entry.str());
                    }
                }
            }
        }

        std::ostringstream details;
        details << "checked steps=" << checkedSteps << " checked seams=" << checkedSeams;
        if (!mismatches.empty()) {
            details << " mismatches=";
            for (std::size_t i = 0; i < mismatches.size(); ++i) {
                if (i > 0) {
                    details << " ; ";
                }
                details << mismatches[i];
            }
        }
        return reportResult("Board neighbor consistency", ok, details.str());
    }

    bool testPawnTopologyDiagnostics() {
        Board board;
        std::ostringstream details;

        const std::array<std::pair<const char*, Board::Direction>, 8> dirs = {{
            {"N", Board::Direction::NORTH},
            {"S", Board::Direction::SOUTH},
            {"E", Board::Direction::EAST},
            {"W", Board::Direction::WEST},
            {"NE", Board::Direction::NORTH_EAST},
            {"SW", Board::Direction::SOUTH_WEST},
            {"SE", Board::Direction::SOUTH_EAST},
            {"NW", Board::Direction::NORTH_WEST}
        }};

        for (const HexCell origin : {HexCell{5, 5}, HexCell{5, 4}, HexCell{1, 4}, HexCell{1, 5}}) {
            details << cellText(origin) << ':';
            for (const auto& [name, dir] : dirs) {
                std::optional<HexCell> step = board.step(origin, dir);
                details << ' ' << name << '=';
                if (step.has_value()) {
                    details << cellText(*step);
                } else {
                    details << "X";
                }
            }
            details << " | ";
        }

        return reportResult("Pawn topology diagnostics", true, details.str());
    }

    bool testAllGeneratedMovesStayValid() {
        Board board;
        PieceStore pieces;
        clearBoard(board);

        addPiece<Rook>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Bishop>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Queen>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<King>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Knight>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER3, {8, 9});

        bool ok = true;
        int moveCount = 0;
        for (const auto& piece : pieces) {
            for (const HexCell& move : piece->getMoves(board)) {
                ++moveCount;
                if (!board.isValid(move)) {
                    ok = false;
                }
            }
        }

        std::ostringstream details;
        details << "checked moves=" << moveCount;
        return reportResult("All generated moves land on valid cells", ok, details.str());
    }

    bool testRookRayAcrossSeam() {
        Board board;
        PieceStore pieces;
        Rook* rook = addPiece<Rook>(board, pieces, Player::PLAYER1, {7, 0});
        const std::vector<HexCell> moves = rook->getMoves(board);
        const bool ok = contains(moves, {4, 11});
        return reportResult("Rook crosses seam through topological ray", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testRookStopsAtFriendlyAndCapturesEnemy() {
        Board board;
        PieceStore pieces;
        Rook* rook = addPiece<Rook>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER1, {1, 6});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {3, 4});
        const std::vector<HexCell> moves = rook->getMoves(board);
        const bool ok = contains(moves, {1, 5}) && !contains(moves, {1, 6}) && contains(moves, {3, 4});
        return reportResult("Rook block/capture behavior", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testBishopCapturesEnemyOnly() {
        Board board;
        PieceStore pieces;
        Bishop* bishop = addPiece<Bishop>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {3, 6});
        addPiece<Pawn>(board, pieces, Player::PLAYER1, {0, 3});
        const std::vector<HexCell> moves = bishop->getMoves(board);
        const bool ok = contains(moves, {3, 6}) && !contains(moves, {0, 3});
        return reportResult("Bishop capture/block behavior", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testBishopCrossesIntoOtherZoneDiagonally() {
        Board board;
        PieceStore pieces;
        Bishop* bishop = addPiece<Bishop>(board, pieces, Player::PLAYER1, {6, 2});
        const std::vector<HexCell> moves = bishop->getMoves(board);
        const bool ok = contains(moves, {8, 4}) && board.getZoneOwner({8, 4}) == Player::PLAYER2;
        return reportResult("Bishop crosses into another side diagonally", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testKnightHasDistinctMoves() {
        Board board;
        PieceStore pieces;
        Knight* knight = addPiece<Knight>(board, pieces, Player::PLAYER1, {1, 4});
        std::vector<HexCell> moves = knight->getMoves(board);
        std::vector<HexCell> uniqueMoves = moves;
        std::sort(uniqueMoves.begin(), uniqueMoves.end(), [](const HexCell& a, const HexCell& b) {
            return a.q < b.q || (a.q == b.q && a.r < b.r);
        });
        uniqueMoves.erase(std::unique(uniqueMoves.begin(), uniqueMoves.end()), uniqueMoves.end());
        const bool ok = !moves.empty() && moves.size() == uniqueMoves.size();
        return reportResult("Knight produces non-duplicate moves", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testKnightCanCaptureEnemyButNotFriendly() {
        Board board;
        PieceStore pieces;
        Knight* knight = addPiece<Knight>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {3, 5});
        addPiece<Pawn>(board, pieces, Player::PLAYER1, {0, 6});
        const std::vector<HexCell> moves = knight->getMoves(board);
        const bool ok = contains(moves, {3, 5}) && !contains(moves, {0, 6});
        return reportResult("Knight capture/friendly exclusion", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testKnightCanLeapAcrossSeam() {
        Board board;
        PieceStore pieces;
        const std::vector<HexCell> candidates = {
            {7, 0}, {7, 1}, {7, 2}, {7, 3},
            {0, 3}, {1, 3}, {2, 3}, {3, 3},
            {8, 7}, {9, 7}, {10, 7}, {11, 7},
            {4, 11}, {5, 11}, {6, 11}, {7, 11},
            {3, 4}, {3, 5}, {3, 6}, {3, 7},
            {11, 8}, {11, 9}, {11, 10}, {11, 11}
        };

        bool ok = false;
        std::string details = "no cross-zone knight leap found";

        for (const HexCell& start : candidates) {
            clearBoard(board);
            pieces.clear();
            Knight* knight = addPiece<Knight>(board, pieces, Player::PLAYER1, start);
            const std::vector<HexCell> moves = knight->getMoves(board);
            for (const HexCell& move : moves) {
                if (board.getZoneOwner(move) != board.getZoneOwner(start)) {
                    ok = true;
                    details = "from " + cellText(start) + " moves: " + moveListText(moves);
                    break;
                }
            }
            (void)knight;
            if (ok) {
                break;
            }
        }

        return reportResult("Knight crosses into another side via seam topology", ok, details);
    }

    bool testKingRejectsFriendlyOccupiedCell() {
        Board board;
        PieceStore pieces;
        King* king = addPiece<King>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER1, {1, 5});
        const std::vector<HexCell> moves = king->getMoves(board);
        const bool ok = !contains(moves, {1, 5});
        return reportResult("King cannot move onto friendly piece", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testKingCapturesEnemyNeighbor() {
        Board board;
        PieceStore pieces;
        King* king = addPiece<King>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {2, 4});
        const std::vector<HexCell> moves = king->getMoves(board);
        const bool ok = contains(moves, {2, 4});
        return reportResult("King can capture enemy neighbor", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testKingStepsAcrossSeam() {
        Board board;
        PieceStore pieces;
        King* king = addPiece<King>(board, pieces, Player::PLAYER1, {7, 0});
        const std::vector<HexCell> moves = king->getMoves(board);
        const bool ok = contains(moves, {4, 11}) && board.getZoneOwner({4, 11}) == Player::PLAYER3;
        return reportResult("King steps across seam into another side", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testPawnForwardAndDoubleStep() {
        Board board;
        PieceStore pieces;
        clearBoard(board);

        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER1, {5, 1});
        std::vector<HexCell> moves = pawn->getMoves(board, nullptr);

        const bool ok = contains(moves, {6, 1}) && contains(moves, {7, 1}) && moves.size() == 2;
        return reportResult("Pawn initial forward + double step", ok, "moves: " + moveListText(moves));
    }

    bool testPawnDiagonalCapture() {
        Board board;
        PieceStore pieces;
        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {2, 5});
        const std::vector<HexCell> moves = pawn->getMoves(board);
        const bool ok = contains(moves, {2, 5});
        return reportResult("Pawn diagonal capture exists", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testPawnSeamTransition() {
        Board board;
        PieceStore pieces;
        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER1, {7, 0});
        const std::vector<HexCell> moves = pawn->getMoves(board);
        const bool ok = contains(moves, {4, 11});
        return reportResult("Pawn seam transition", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testPawnContinuesAfterSeamTransition() {
        Board board;
        PieceStore pieces;
        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER1, {4, 11});
        pawn->setHasMoved(true);
        const std::vector<HexCell> moves = pawn->getMoves(board);
        const bool ok = contains(moves, {4, 10}) && board.getZoneOwner({4, 10}) == Player::PLAYER3;
        return reportResult("Pawn continues away from enemy base after seam transition", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testPawnBlockedForwardCannotAdvance() {
        Board board;
        PieceStore pieces;
        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {1, 5});
        const std::vector<HexCell> moves = pawn->getMoves(board);
        const bool ok = !contains(moves, {1, 5}) && !contains(moves, {1, 6});
        return reportResult("Pawn blocked forward cannot advance", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testQueenCombinesRookAndBishopPatterns() {
        Board board;
        PieceStore pieces;
        Queen* queen = addPiece<Queen>(board, pieces, Player::PLAYER1, {1, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {3, 4});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {3, 6});
        const std::vector<HexCell> moves = queen->getMoves(board);
        const bool ok = contains(moves, {3, 4}) && contains(moves, {3, 6}) && contains(moves, {1, 5});
        return reportResult("Queen combines straight and diagonal reach", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testQueenCrossesSeamThroughRay() {
        Board board;
        PieceStore pieces;
        Queen* queen = addPiece<Queen>(board, pieces, Player::PLAYER1, {7, 0});
        const std::vector<HexCell> moves = queen->getMoves(board);
        const bool ok = contains(moves, {4, 11}) && board.getZoneOwner({4, 11}) == Player::PLAYER3;
        return reportResult("Queen crosses seam through topological ray", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testPromotionZonesReachability() {
        Board board;
        bool ok = true;
        ok = ok && board.isPromotionCell({4, 8}, Player::PLAYER1);
        ok = ok && board.isPromotionCell({0, 0}, Player::PLAYER2);
        ok = ok && board.isPromotionCell({8, 4}, Player::PLAYER3);
        ok = ok && !board.isPromotionCell({1, 4}, Player::PLAYER1);
        return reportResult("Promotion zone invariants", ok,
            "sample promotion cells verified");
    }

    bool testPawnEnPassantPattern() {
        Board board;
        PieceStore pieces;
        Pawn* pawn = addPiece<Pawn>(board, pieces, Player::PLAYER3, {0, 6});
        addPiece<Pawn>(board, pieces, Player::PLAYER2, {2, 5});
        Move lastMove;
        lastMove.from = {0, 5};
        lastMove.to = {2, 5};
        lastMove.player = Player::PLAYER2;
        const std::vector<HexCell> moves = pawn->getMoves(board, &lastMove);
        const bool ok = contains(moves, {1, 5});
        return reportResult("Pawn en passant pattern", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testInitialBoardHasMoves() {
        Board board;
        PieceStore pieces;
        clearBoard(board);

        auto placeInitial = [&](Player player) {
            if (player == Player::PLAYER1) {
                addPiece<Rook>(board, pieces, player, {0, 0});
                addPiece<Knight>(board, pieces, player, {1, 0});
                addPiece<Bishop>(board, pieces, player, {2, 0});
                addPiece<Queen>(board, pieces, player, {3, 0});
                addPiece<Rook>(board, pieces, player, {4, 0});
                addPiece<Pawn>(board, pieces, player, {5, 0});
                addPiece<Pawn>(board, pieces, player, {0, 1});
                addPiece<Pawn>(board, pieces, player, {1, 1});
                addPiece<Pawn>(board, pieces, player, {2, 1});
                addPiece<Pawn>(board, pieces, player, {3, 1});
                addPiece<Knight>(board, pieces, player, {4, 1});
                addPiece<Pawn>(board, pieces, player, {5, 1});
                addPiece<Bishop>(board, pieces, player, {4, 2});
                addPiece<Pawn>(board, pieces, player, {5, 2});
                addPiece<King>(board, pieces, player, {4, 3});
                addPiece<Pawn>(board, pieces, player, {5, 3});
            } else if (player == Player::PLAYER2) {
                addPiece<Rook>(board, pieces, player, {0, 4});
                addPiece<Pawn>(board, pieces, player, {1, 4});
                addPiece<Rook>(board, pieces, player, {8, 4});
                addPiece<Knight>(board, pieces, player, {9, 4});
                addPiece<Bishop>(board, pieces, player, {10, 4});
                addPiece<Queen>(board, pieces, player, {11, 4});
                addPiece<Knight>(board, pieces, player, {0, 5});
                addPiece<Pawn>(board, pieces, player, {1, 5});
                addPiece<Pawn>(board, pieces, player, {8, 5});
                addPiece<Pawn>(board, pieces, player, {9, 5});
                addPiece<Pawn>(board, pieces, player, {10, 5});
                addPiece<Pawn>(board, pieces, player, {11, 5});
                addPiece<Bishop>(board, pieces, player, {0, 6});
                addPiece<Pawn>(board, pieces, player, {1, 6});
                addPiece<King>(board, pieces, player, {0, 7});
                addPiece<Pawn>(board, pieces, player, {1, 7});
            } else {
                addPiece<Rook>(board, pieces, player, {4, 8});
                addPiece<Knight>(board, pieces, player, {5, 8});
                addPiece<Bishop>(board, pieces, player, {6, 8});
                addPiece<Queen>(board, pieces, player, {7, 8});
                addPiece<Rook>(board, pieces, player, {8, 8});
                addPiece<Pawn>(board, pieces, player, {9, 8});
                addPiece<Pawn>(board, pieces, player, {4, 9});
                addPiece<Pawn>(board, pieces, player, {5, 9});
                addPiece<Pawn>(board, pieces, player, {6, 9});
                addPiece<Pawn>(board, pieces, player, {7, 9});
                addPiece<Knight>(board, pieces, player, {8, 9});
                addPiece<Pawn>(board, pieces, player, {9, 9});
                addPiece<Bishop>(board, pieces, player, {8, 10});
                addPiece<Pawn>(board, pieces, player, {9, 10});
                addPiece<King>(board, pieces, player, {8, 11});
                addPiece<Pawn>(board, pieces, player, {9, 11});
            }
        };

        placeInitial(Player::PLAYER1);
        placeInitial(Player::PLAYER2);
        placeInitial(Player::PLAYER3);

        int movablePieces = 0;
        for (const auto& piece : pieces) {
            if (!piece->getMoves(board).empty()) {
                ++movablePieces;
            }
        }

        const bool ok = movablePieces > 0;
        std::ostringstream details;
        details << "movable pieces: " << movablePieces << '/' << pieces.size();
        return reportResult("Initial setup has playable pieces", ok, details.str());
    }

    bool testInitialPlayablePiecesByPlayer() {
        Board board;
        PieceStore pieces;
        clearBoard(board);

        auto placeInitial = [&](Player player) {
            if (player == Player::PLAYER1) {
                addPiece<Rook>(board, pieces, player, {0, 0});
                addPiece<Knight>(board, pieces, player, {1, 0});
                addPiece<Bishop>(board, pieces, player, {2, 0});
                addPiece<Queen>(board, pieces, player, {3, 0});
                addPiece<Rook>(board, pieces, player, {4, 0});
                addPiece<Pawn>(board, pieces, player, {5, 0});
                addPiece<Pawn>(board, pieces, player, {0, 1});
                addPiece<Pawn>(board, pieces, player, {1, 1});
                addPiece<Pawn>(board, pieces, player, {2, 1});
                addPiece<Pawn>(board, pieces, player, {3, 1});
                addPiece<Knight>(board, pieces, player, {4, 1});
                addPiece<Pawn>(board, pieces, player, {5, 1});
                addPiece<Bishop>(board, pieces, player, {4, 2});
                addPiece<Pawn>(board, pieces, player, {5, 2});
                addPiece<King>(board, pieces, player, {4, 3});
                addPiece<Pawn>(board, pieces, player, {5, 3});
            } else if (player == Player::PLAYER2) {
                addPiece<Rook>(board, pieces, player, {0, 4});
                addPiece<Pawn>(board, pieces, player, {1, 4});
                addPiece<Rook>(board, pieces, player, {8, 4});
                addPiece<Knight>(board, pieces, player, {9, 4});
                addPiece<Bishop>(board, pieces, player, {10, 4});
                addPiece<Queen>(board, pieces, player, {11, 4});
                addPiece<Knight>(board, pieces, player, {0, 5});
                addPiece<Pawn>(board, pieces, player, {1, 5});
                addPiece<Pawn>(board, pieces, player, {8, 5});
                addPiece<Pawn>(board, pieces, player, {9, 5});
                addPiece<Pawn>(board, pieces, player, {10, 5});
                addPiece<Pawn>(board, pieces, player, {11, 5});
                addPiece<Bishop>(board, pieces, player, {0, 6});
                addPiece<Pawn>(board, pieces, player, {1, 6});
                addPiece<King>(board, pieces, player, {0, 7});
                addPiece<Pawn>(board, pieces, player, {1, 7});
            } else {
                addPiece<Rook>(board, pieces, player, {4, 8});
                addPiece<Knight>(board, pieces, player, {5, 8});
                addPiece<Bishop>(board, pieces, player, {6, 8});
                addPiece<Queen>(board, pieces, player, {7, 8});
                addPiece<Rook>(board, pieces, player, {8, 8});
                addPiece<Pawn>(board, pieces, player, {9, 8});
                addPiece<Pawn>(board, pieces, player, {4, 9});
                addPiece<Pawn>(board, pieces, player, {5, 9});
                addPiece<Pawn>(board, pieces, player, {6, 9});
                addPiece<Pawn>(board, pieces, player, {7, 9});
                addPiece<Knight>(board, pieces, player, {8, 9});
                addPiece<Pawn>(board, pieces, player, {9, 9});
                addPiece<Bishop>(board, pieces, player, {8, 10});
                addPiece<Pawn>(board, pieces, player, {9, 10});
                addPiece<King>(board, pieces, player, {8, 11});
                addPiece<Pawn>(board, pieces, player, {9, 11});
            }
        };

        placeInitial(Player::PLAYER1);
        placeInitial(Player::PLAYER2);
        placeInitial(Player::PLAYER3);

        std::ostringstream details;
        bool ok = true;

        for (Player player : {Player::PLAYER1, Player::PLAYER2, Player::PLAYER3}) {
            int movableCount = 0;
            details << playerText(player) << ':';
            for (const auto& piece : pieces) {
                if (piece->getOwner() != player) {
                    continue;
                }

                std::vector<HexCell> moves;
                if (piece->getType() == PieceType::PAWN) {
                    const Pawn* pawn = dynamic_cast<const Pawn*>(piece.get());
                    moves = pawn ? pawn->getMoves(board, nullptr) : piece->getMoves(board);
                } else {
                    moves = piece->getMoves(board);
                }

                if (!moves.empty()) {
                    ++movableCount;
                    details << ' ' << pieceText(piece->getType()) << cellText(piece->getPos())
                            << '=' << moves.size();
                }
            }

            if (movableCount == 0) {
                ok = false;
                details << " none";
            }
            details << " | ";
        }

        return reportResult("Initial playable pieces by player", ok, details.str());
    }

    bool testScenarioTurnOrderCyclesCorrectly() {
        GameState state;
        initStandardPosition(state);

        const Player p0 = state.getCurrentPlayer();
        state.applyMove({{5, 1}, {7, 1}, Player::PLAYER1});
        const Player p1 = state.getCurrentPlayer();
        state.applyMove({{1, 4}, {1, 6}, Player::PLAYER2});
        const Player p2 = state.getCurrentPlayer();
        state.applyMove({{9, 8}, {9, 10}, Player::PLAYER3});
        const Player p3 = state.getCurrentPlayer();

        const bool ok = p0 == Player::PLAYER1 && p1 == Player::PLAYER2 && p2 == Player::PLAYER3 && p3 == Player::PLAYER1;
        return reportResult("Scenario turn order cycles correctly", ok,
            "P1 -> P2 -> P3 -> P1");
    }

    bool testScenarioPawnSequenceAppliesOnBoard() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{5, 1}, {7, 1}, Player::PLAYER1});
        state.applyMove({{1, 4}, {1, 6}, Player::PLAYER2});
        state.applyMove({{9, 8}, {9, 10}, Player::PLAYER3});

        Piece* whitePawn = state.getBoard().getPiece({7, 1});
        Piece* bluePawn = state.getBoard().getPiece({1, 6});
        Piece* redPawn = state.getBoard().getPiece({9, 10});

        const bool ok = whitePawn && whitePawn->getOwner() == Player::PLAYER1 &&
                        bluePawn && bluePawn->getOwner() == Player::PLAYER2 &&
                        redPawn && redPawn->getOwner() == Player::PLAYER3 &&
                        state.getBoard().getPiece({5, 1}) == nullptr &&
                        state.getBoard().getPiece({1, 4}) == nullptr &&
                        state.getBoard().getPiece({9, 8}) == nullptr;
        return reportResult("Scenario pawn sequence updates board state", ok,
            "white (7,1), blue (1,6), red (9,10)");
    }

    bool testScenarioRookBecomesPlayableAfterPawnMove() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{1, 4}, {1, 6}, Player::PLAYER2});

        Piece* rookPiece = state.getBoard().getPiece({0, 4});
        const auto moves = rookPiece ? rookPiece->getMoves(state.getBoard()) : std::vector<HexCell>{};
        const bool ok = rookPiece && rookPiece->getType() == PieceType::ROOK && contains(moves, {1, 4}) && contains(moves, {2, 4});
        return reportResult("Scenario rook becomes playable after pawn move", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testScenarioBishopOpensAfterPawnSequence() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{5, 2}, {7, 2}, Player::PLAYER1});

        Piece* bishopPiece = state.getBoard().getPiece({4, 2});
        const auto moves = bishopPiece ? bishopPiece->getMoves(state.getBoard()) : std::vector<HexCell>{};
        const bool ok = bishopPiece && bishopPiece->getType() == PieceType::BISHOP && !moves.empty();
        return reportResult("Scenario bishop opens after pawn sequence", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testScenarioQueenOpensAfterCentralPawnMove() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{3, 1}, {3, 3}, Player::PLAYER1});

        Piece* queenPiece = state.getBoard().getPiece({3, 0});
        const auto moves = queenPiece ? queenPiece->getMoves(state.getBoard()) : std::vector<HexCell>{};
        const bool ok = queenPiece && queenPiece->getType() == PieceType::QUEEN && contains(moves, {3, 1});
        return reportResult("Scenario queen opens after pawn move", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testScenarioKnightRemainsMobileInOpening() {
        GameState state;
        initStandardPosition(state);

        Piece* knightPiece = state.getBoard().getPiece({4, 1});
        const auto moves = knightPiece ? knightPiece->getMoves(state.getBoard()) : std::vector<HexCell>{};
        const bool ok = knightPiece && knightPiece->getType() == PieceType::KNIGHT && moves.size() >= 2;
        return reportResult("Scenario knight is mobile in opening position", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testScenarioKingGainsNeighborAfterPawnMove() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{5, 3}, {4, 11}, Player::PLAYER1});

        Piece* kingPiece = state.getBoard().getPiece({4, 3});
        const auto moves = kingPiece ? kingPiece->getMoves(state.getBoard()) : std::vector<HexCell>{};
        const bool ok = kingPiece && kingPiece->getType() == PieceType::KING && !moves.empty();
        return reportResult("Scenario king gains legal neighbors after pawn move", ok,
            std::string("moves: ") + moveListText(moves));
    }

    bool testScenarioPawnCrossesSeamInRealGame() {
        GameState state;
        initStandardPosition(state);

        state.applyMove({{5, 3}, {4, 11}, Player::PLAYER1});

        Piece* piece = state.getBoard().getPiece({4, 11});
        const Move* lastMove = state.getLastMove();
        const bool ok = piece && piece->getType() == PieceType::QUEEN && piece->getOwner() == Player::PLAYER1 &&
                        lastMove && lastMove->to == HexCell{4, 11} && lastMove->from == HexCell{5, 3} && lastMove->isPromotion;
        return reportResult("Scenario pawn crosses seam in live game", ok,
            piece ? "pawn crossed seam and promoted on (4,11)" : "piece missing after seam move");
    }

    bool testScenarioCaptureUpdatesBoardCorrectly() {
        GameState state;
        clearBoard(state.getBoard());

        PieceFactory factory;
        state.getBoard().setPiece({1, 4}, factory.create(PieceType::ROOK, Player::PLAYER1, {1, 4}));
        state.getBoard().setPiece({3, 4}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 4}));

        state.applyMove({{1, 4}, {3, 4}, Player::PLAYER1});

        Piece* pieceAtTarget = state.getBoard().getPiece({3, 4});
        const bool ok = pieceAtTarget && pieceAtTarget->getType() == PieceType::ROOK && pieceAtTarget->getOwner() == Player::PLAYER1 &&
                        state.getBoard().getPiece({1, 4}) == nullptr;
        return reportResult("Scenario capture updates board correctly", ok,
            "rook captured on (3,4)");
    }

    bool testUndoMoveRoundTripSimpleMove() {
        GameState state;
        clearBoard(state.getBoard());

        PieceFactory factory;
        state.getBoard().setPiece({1, 4}, factory.create(PieceType::ROOK, Player::PLAYER1, {1, 4}));

        const auto before = snapshotBoard(state.getBoard());
        state.applyMove({{1, 4}, {3, 4}, Player::PLAYER1});
        state.undoMove();

        std::string mismatch;
        const bool boardOk = boardMatchesSnapshot(state.getBoard(), before, mismatch);
        Piece* rook = state.getBoard().getPiece({1, 4});
        const bool ok = boardOk && rook && rook->getType() == PieceType::ROOK &&
                        rook->getOwner() == Player::PLAYER1 && !rook->getHasMoved() &&
                        state.getBoard().getPiece({3, 4}) == nullptr;
        return reportResult("Undo round-trip simple move", ok,
            ok ? "rook restored to (1,4)" : mismatch);
    }

    bool testUndoMoveRoundTripCapture() {
        GameState state;
        clearBoard(state.getBoard());

        PieceFactory factory;
        state.getBoard().setPiece({1, 4}, factory.create(PieceType::ROOK, Player::PLAYER1, {1, 4}));
        state.getBoard().setPiece({3, 4}, factory.create(PieceType::PAWN, Player::PLAYER2, {3, 4}));

        const auto before = snapshotBoard(state.getBoard());
        state.applyMove({{1, 4}, {3, 4}, Player::PLAYER1});
        state.undoMove();

        std::string mismatch;
        const bool boardOk = boardMatchesSnapshot(state.getBoard(), before, mismatch);
        Piece* rook = state.getBoard().getPiece({1, 4});
        Piece* pawn = state.getBoard().getPiece({3, 4});
        const bool ok = boardOk && rook && rook->getType() == PieceType::ROOK &&
                        rook->getOwner() == Player::PLAYER1 && pawn &&
                        pawn->getType() == PieceType::PAWN && pawn->getOwner() == Player::PLAYER2;
        return reportResult("Undo round-trip capture", ok,
            ok ? "captured pawn restored" : mismatch);
    }

    bool testUndoMoveRoundTripEnPassant() {
        GameState state;
        clearBoard(state.getBoard());

        PieceFactory factory;
        state.getBoard().setPiece({0, 6}, factory.create(PieceType::PAWN, Player::PLAYER3, {0, 6}));
        state.getBoard().setPiece({2, 5}, factory.create(PieceType::PAWN, Player::PLAYER2, {2, 5}));

        Move lastMove;
        lastMove.from = {0, 5};
        lastMove.to = {2, 5};
        lastMove.player = Player::PLAYER2;
        state.applyMove(lastMove);
        state.undoMove();

        state.getBoard().removePiece({0, 5});
        state.getBoard().setPiece({2, 5}, factory.create(PieceType::PAWN, Player::PLAYER2, {2, 5}));

        const auto before = snapshotBoard(state.getBoard());
        state.applyMove({{0, 6}, {1, 5}, Player::PLAYER3});
        state.undoMove();

        std::string mismatch;
        const bool boardOk = boardMatchesSnapshot(state.getBoard(), before, mismatch);
        Piece* capturingPawn = state.getBoard().getPiece({0, 6});
        Piece* capturedPawn = state.getBoard().getPiece({2, 5});
        const bool ok = boardOk && capturingPawn && capturingPawn->getType() == PieceType::PAWN &&
                        capturingPawn->getOwner() == Player::PLAYER3 && capturedPawn &&
                        capturedPawn->getType() == PieceType::PAWN && capturedPawn->getOwner() == Player::PLAYER2;
        return reportResult("Undo round-trip en passant", ok,
            ok ? "both pawns restored" : mismatch);
    }

    bool testUndoMoveRoundTripPromotion() {
        GameState state;
        clearBoard(state.getBoard());

        PieceFactory factory;
        state.getBoard().setPiece({5, 3}, factory.create(PieceType::PAWN, Player::PLAYER1, {5, 3}));

        const auto before = snapshotBoard(state.getBoard());
        state.applyMove({{5, 3}, {4, 11}, Player::PLAYER1});
        Piece* promoted = state.getBoard().getPiece({4, 11});
        const bool promotedOk = promoted && promoted->getType() == PieceType::QUEEN && promoted->getOwner() == Player::PLAYER1;
        state.undoMove();

        std::string mismatch;
        const bool boardOk = boardMatchesSnapshot(state.getBoard(), before, mismatch);
        Piece* pawn = state.getBoard().getPiece({5, 3});
        const bool ok = promotedOk && boardOk && pawn && pawn->getType() == PieceType::PAWN &&
                        pawn->getOwner() == Player::PLAYER1 && state.getBoard().getPiece({4, 11}) == nullptr;
        return reportResult("Undo round-trip promotion", ok,
            ok ? "queen removed and pawn restored" : mismatch);
    }
}

int main() {
    const std::vector<TestCase> tests = {
        {"Board seam transitions", testBoardStepSeams},
        {"Board diagonal seam transitions", testBoardDiagonalSeamTransitions},
        {"Board validity map matches 96-cell layout", testBoardValidityMap},
        {"Board neighbor consistency", testBoardNeighborSymmetry},
        {"Pawn topology diagnostics", testPawnTopologyDiagnostics},
        {"All generated moves land on valid cells", testAllGeneratedMovesStayValid},
        {"Rook crosses seam through topological ray", testRookRayAcrossSeam},
        {"Rook block/capture behavior", testRookStopsAtFriendlyAndCapturesEnemy},
        {"Bishop capture/block behavior", testBishopCapturesEnemyOnly},
        {"Bishop crosses into another side diagonally", testBishopCrossesIntoOtherZoneDiagonally},
        {"Knight produces non-duplicate moves", testKnightHasDistinctMoves},
        {"Knight capture/friendly exclusion", testKnightCanCaptureEnemyButNotFriendly},
        {"Knight crosses into another side via seam topology", testKnightCanLeapAcrossSeam},
        {"King cannot move onto friendly piece", testKingRejectsFriendlyOccupiedCell},
        {"King can capture enemy neighbor", testKingCapturesEnemyNeighbor},
        {"King steps across seam into another side", testKingStepsAcrossSeam},
        {"Pawn initial forward + double step", testPawnForwardAndDoubleStep},
        {"Pawn diagonal capture exists", testPawnDiagonalCapture},
        {"Pawn seam transition", testPawnSeamTransition},
        {"Pawn continues away from enemy base after seam transition", testPawnContinuesAfterSeamTransition},
        {"Pawn blocked forward cannot advance", testPawnBlockedForwardCannotAdvance},
        {"Queen combines straight and diagonal reach", testQueenCombinesRookAndBishopPatterns},
        {"Queen crosses seam through topological ray", testQueenCrossesSeamThroughRay},
        {"Promotion zone invariants", testPromotionZonesReachability},
        {"Pawn en passant pattern", testPawnEnPassantPattern},
        {"Initial setup has playable pieces", testInitialBoardHasMoves},
        {"Initial playable pieces by player", testInitialPlayablePiecesByPlayer},
        {"Scenario turn order cycles correctly", testScenarioTurnOrderCyclesCorrectly},
        {"Scenario pawn sequence updates board state", testScenarioPawnSequenceAppliesOnBoard},
        {"Scenario rook becomes playable after pawn move", testScenarioRookBecomesPlayableAfterPawnMove},
        {"Scenario bishop opens after pawn sequence", testScenarioBishopOpensAfterPawnSequence},
        {"Scenario queen opens after pawn move", testScenarioQueenOpensAfterCentralPawnMove},
        {"Scenario knight is mobile in opening position", testScenarioKnightRemainsMobileInOpening},
        {"Scenario king gains legal neighbors after pawn move", testScenarioKingGainsNeighborAfterPawnMove},
        {"Scenario pawn crosses seam in live game", testScenarioPawnCrossesSeamInRealGame},
        {"Scenario capture updates board correctly", testScenarioCaptureUpdatesBoardCorrectly},
        {"Undo round-trip simple move", testUndoMoveRoundTripSimpleMove},
        {"Undo round-trip capture", testUndoMoveRoundTripCapture},
        {"Undo round-trip en passant", testUndoMoveRoundTripEnPassant},
        {"Undo round-trip promotion", testUndoMoveRoundTripPromotion}
    };

    std::cout << "Manual game logic tests\n\n";

    int passed = 0;
    for (const TestCase& test : tests) {
        if (test.run()) {
            ++passed;
        }
    }

    std::cout << "\nSummary: " << passed << '/' << tests.size() << " tests passed.\n";
    return passed == static_cast<int>(tests.size()) ? 0 : 1;
}
