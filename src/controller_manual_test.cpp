#include "Controller/GameController.hpp"

#include <array>
#include <functional>
#include <iostream>
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
            case Player::PLAYER1: return "Joueur 1";
            case Player::PLAYER2: return "Joueur 2";
            case Player::PLAYER3: return "Joueur 3";
            default: return "Aucun";
        }
    }

    bool reportResult(const std::string& name, bool ok, const std::string& details) {
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << '\n';
        if (!details.empty()) {
            std::cout << "       " << details << '\n';
        }
        return ok;
    }

    bool testControllerStoresAIConfig() {
        GameController controller(false);
        const std::array<bool, 3> expected = {true, false, true};
        controller.setAIConfig(expected);
        const bool ok = controller.getAIConfig() == expected;
        return reportResult("Controller stores AI config", ok,
            ok ? "configuration persisted" : "configuration mismatch after setAIConfig");
    }

    bool testControllerCanStartGameForTests() {
        GameController controller(false);
        controller.setAIConfig({false, false, false});
        controller.startGameForTests();
        const bool ok = controller.hasGameStarted();
        return reportResult("Controller can start game without menu", ok,
            ok ? "gameStarted=true" : "gameStarted stayed false");
    }

    bool testAllAIControllerStepAdvancesTurn() {
        GameController controller(false);
        controller.setAIConfig({true, true, true});
        controller.startGameForTests();

        const Player before = controller.getState().getCurrentPlayer();
        const bool moved = controller.stepAIMoveForTests(true);
        const Player after = controller.getState().getCurrentPlayer();
        const bool ok = moved && after != before;

        std::ostringstream details;
        details << "before=" << playerText(before)
                << " after=" << playerText(after)
                << " moved=" << (moved ? "yes" : "no");
        return reportResult("All-AI controller step advances turn", ok, details.str());
    }

    bool testMixedConfigDoesNotMoveForHumanTurn() {
        GameController controller(false);
        controller.setAIConfig({false, true, true});
        controller.startGameForTests();

        const Player before = controller.getState().getCurrentPlayer();
        const bool moved = controller.stepAIMoveForTests(true);
        const Player after = controller.getState().getCurrentPlayer();
        const bool ok = !moved && after == before;

        std::ostringstream details;
        details << "current=" << playerText(before)
                << " moved=" << (moved ? "yes" : "no");
        return reportResult("Human turn is not auto-played in mixed config", ok, details.str());
    }
}

int main() {
    const std::vector<TestCase> tests = {
        {"Controller stores AI config", testControllerStoresAIConfig},
        {"Controller can start game without menu", testControllerCanStartGameForTests},
        {"All-AI controller step advances turn", testAllAIControllerStepAdvancesTurn},
        {"Human turn is not auto-played in mixed config", testMixedConfigDoesNotMoveForHumanTurn}
    };

    std::cout << "Controller manual tests\n\n";

    int passed = 0;
    for (const TestCase& test : tests) {
        if (test.run()) {
            ++passed;
        }
    }

    std::cout << "\nSummary: " << passed << '/' << tests.size() << " tests passed.\n";
    return passed == static_cast<int>(tests.size()) ? 0 : 1;
}
