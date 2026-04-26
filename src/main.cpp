#include "View/SetupScreen.hpp"
#include "Controller/GameController.hpp"
#include "Debug/BoardDebug.hpp"

int main() {
    SetupScreen setup;
    GameConfig config = setup.run();
    if (!setup.wasStarted()) return 0;

    GameController game(config);
    debug::printBoardLayout(game.getState().getBoard());  // TODO: remove after layout confirmed
    game.run();
    return 0;
}
