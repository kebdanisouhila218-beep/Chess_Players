#include "Model/GameState.hpp"
#include "Model/PieceFactory.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {
    struct TestCase {
        std::string name;
        std::function<bool()> run;
    };

    std::string playerText(Player player) {
        switch (player) {
            case Player::PLAYER1: return "Joueur 1 - Blancs";
            case Player::PLAYER2: return "Joueur 2 - Bleus";
            case Player::PLAYER3: return "Joueur 3 - Rouges";
            default: return "Aucun";
        }
    }

    std::string cellText(const HexCell& cell) {
        std::ostringstream out;
        out << '(' << cell.q << ',' << cell.r << ')';
        return out.str();
    }

    std::string moveText(const Move& move) {
        std::ostringstream out;
        out << playerText(move.player) << ' ' << cellText(move.from) << " -> " << cellText(move.to);
        return out.str();
    }

    void initStandardPosition(GameState& state) {
        PieceFactory factory;
        factory.initBoard(state.getBoard(), Player::PLAYER1);
        factory.initBoard(state.getBoard(), Player::PLAYER2);
        factory.initBoard(state.getBoard(), Player::PLAYER3);
    }

    int playerIndex(Player player) {
        switch (player) {
            case Player::PLAYER1: return 0;
            case Player::PLAYER2: return 1;
            case Player::PLAYER3: return 2;
            default: return -1;
        }
    }

    bool reportResult(const std::string& name, bool ok, const std::string& details) {
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << '\n';
        if (!details.empty()) {
            std::cout << "       " << details << '\n';
        }
        return ok;
    }

    void printManualMenuChecklist() {
        std::cout << "================ Menu / IA manual checklist ================\n";
        std::cout << "1. Lancer Chess3Players.exe et verifier que le menu s'affiche avant la partie.\n";
        std::cout << "2. Cliquer sur chaque bouton Joueur 1/2/3 et verifier le changement visuel Humain <-> IA.\n";
        std::cout << "3. Fermer la fenetre depuis le menu et verifier qu'il n'y a ni crash ni blocage.\n";
        std::cout << "4. Reouvrir le jeu, choisir Humain/Humain/Humain puis verifier qu'aucun coup auto n'est joue.\n";
        std::cout << "5. Reouvrir le jeu, choisir IA/IA/IA puis verifier que la partie avance sans retour visuel de simulation.\n";
        std::cout << "6. Reouvrir le jeu, choisir Humain/IA/Humain puis jouer un coup blanc pour verifier que le bleu joue tout seul.\n";
        std::cout << "=============================================================\n\n";
    }

    bool testAllAIProgressesForSeveralTurns() {
        GameState state;
        initStandardPosition(state);

        std::vector<std::string> playedMoves;
        std::set<Player> seenPlayers;
        bool ok = true;

        for (int turn = 0; turn < 6; ++turn) {
            const Player current = state.getCurrentPlayer();
            seenPlayers.insert(current);

            std::optional<Move> bestMove = state.findBestMove(2, current);
            if (!bestMove.has_value()) {
                ok = false;
                playedMoves.push_back("Aucun coup trouve pour " + playerText(current));
                break;
            }

            playedMoves.push_back(moveText(*bestMove));
            state.applyMove(*bestMove);
        }

        std::ostringstream details;
        details << "moves=" << playedMoves.size() << " seenPlayers=" << seenPlayers.size();
        for (const std::string& move : playedMoves) {
            details << " | " << move;
        }

        return reportResult("All-AI scenario progresses for several turns",
            ok && playedMoves.size() == 6 && seenPlayers.size() >= 3,
            details.str());
    }

    bool testMixedHumanThenAIResponds() {
        GameState state;
        initStandardPosition(state);

        const std::array<bool, 3> isAI = {false, true, false};
        const Player openingPlayer = state.getCurrentPlayer();
        std::optional<Move> humanMove;

        for (const HexCell& cell : state.getBoard().allValidCells()) {
            Piece* piece = state.getBoard().getPiece(cell);
            if (!piece || piece->getOwner() != openingPlayer) {
                continue;
            }
            std::vector<Move> legalMoves = state.getLegalMovesAsMove(cell);
            if (!legalMoves.empty()) {
                humanMove = legalMoves.front();
                break;
            }
        }

        if (!humanMove.has_value()) {
            return reportResult("Mixed Human/AI scenario has an opening human move", false,
                "Aucun coup humain legal trouve pour le joueur initial");
        }

        const std::string humanMoveText = moveText(*humanMove);
        state.applyMove(*humanMove);
        const Player afterHuman = state.getCurrentPlayer();
        const int aiIndex = playerIndex(afterHuman);

        bool aiConfigured = aiIndex >= 0 && isAI[static_cast<std::size_t>(aiIndex)];
        std::optional<Move> aiMove;
        if (aiConfigured) {
            aiMove = state.findBestMove(2, afterHuman);
        }

        std::ostringstream details;
        details << "human=" << humanMoveText
                << " | next=" << playerText(afterHuman)
                << " | aiConfigured=" << (aiConfigured ? "yes" : "no");
        if (aiMove.has_value()) {
            details << " | ai=" << moveText(*aiMove);
        }

        return reportResult("Mixed Human/AI scenario yields an AI response candidate",
            aiConfigured && aiMove.has_value(),
            details.str());
    }

    bool testAllAISequenceKeepsCurrentPlayerAdvancing() {
        GameState state;
        initStandardPosition(state);

        std::vector<Player> order;
        bool ok = true;

        for (int turn = 0; turn < 5; ++turn) {
            const Player before = state.getCurrentPlayer();
            order.push_back(before);

            std::optional<Move> bestMove = state.findBestMove(2, before);
            if (!bestMove.has_value()) {
                ok = false;
                break;
            }

            state.applyMove(*bestMove);
            const Player after = state.getCurrentPlayer();
            if (after == before) {
                ok = false;
                break;
            }
        }

        std::ostringstream details;
        for (std::size_t i = 0; i < order.size(); ++i) {
            if (i > 0) {
                details << " -> ";
            }
            details << playerText(order[i]);
        }

        return reportResult("All-AI scenario advances the current player after each move", ok, details.str());
    }
}

int main() {
    printManualMenuChecklist();

    const std::vector<TestCase> tests = {
        {"All-AI scenario progresses for several turns", testAllAIProgressesForSeveralTurns},
        {"Mixed Human/AI scenario yields an AI response candidate", testMixedHumanThenAIResponds},
        {"All-AI scenario advances the current player after each move", testAllAISequenceKeepsCurrentPlayerAdvancing}
    };

    std::cout << "Menu / AI guided manual tests\n\n";

    int passed = 0;
    for (const TestCase& test : tests) {
        if (test.run()) {
            ++passed;
        }
    }

    std::cout << "\nSummary: " << passed << '/' << tests.size() << " checks passed.\n";
    std::cout << "Use this executable together with Chess3Players.exe for the UI-side manual verification.\n";
    return passed == static_cast<int>(tests.size()) ? 0 : 1;
}
