#pragma once
#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "../Model/GameState.hpp"
#include "../Model/GameConfig.hpp"
#include "../Model/PieceFactory.hpp"
#include "../View/MenuRenderer.hpp"
#include "../View/Renderer.hpp"

class GameController {
public:
    // Production constructor: takes config from SetupScreen, skips internal menu
    explicit GameController(const GameConfig& config, bool windowVisible = true);

    // Legacy constructor: shows built-in menu (used by tests)
    explicit GameController(bool windowVisible = true);

    void run();

    // Test interface
    void setAIConfig(const std::array<bool, 3>& config);
    const std::array<bool, 3>& getAIConfig() const;
    void setAIDifficulty(int depth);
    void startGameForTests();
    bool stepAIMoveForTests(bool ignoreDelay = true);
    bool hasGameStarted() const;
    GameState& getState();
    const GameState& getState() const;

private:
    void showMenu();
    void startGame();
    void handleEvents();
    void handleClick(int x, int y);
    void handleMenuClick(int x, int y);
    bool tryAIMove(bool ignoreDelay = false);
    void updateStatusMessage(const std::string& yaltaMessage = "");
    void handleEndScreenClick(int x, int y);
    std::string pieceLabel(const Piece* piece, const HexCell& cell) const;

    sf::RenderWindow window;
    GameState        state;
    MenuRenderer     menuRenderer;
    Renderer         renderer;
    PieceFactory     factory;

    GameConfig          m_config;
    std::array<bool, 3> isAI    = {false, false, true};
    int                 aiDepth = 2;

    bool gameStarted = false;
    std::chrono::steady_clock::time_point lastAIMoveTime = std::chrono::steady_clock::now();
    std::chrono::milliseconds aiMoveDelay{1400};

    HexCell* selected = nullptr;
    std::vector<HexCell> validMoves;
};
