#pragma once
#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "../Model/GameState.hpp"
#include "../Model/PieceFactory.hpp"
#include "../View/MenuRenderer.hpp"
#include "../View/Renderer.hpp"

class GameController {
public:
    explicit GameController(bool windowVisible = true);
    void run();
    void setAIConfig(const std::array<bool, 3>& config);
    const std::array<bool, 3>& getAIConfig() const;
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
    std::string pieceLabel(const Piece* piece, const HexCell& cell) const;

    sf::RenderWindow window;
    GameState        state;
    MenuRenderer     menuRenderer;
    Renderer         renderer;
    PieceFactory     factory;
    std::array<bool, 3> isAI = {false, false, true};
    bool gameStarted = false;
    std::chrono::steady_clock::time_point lastAIMoveTime = std::chrono::steady_clock::now();
    std::chrono::milliseconds aiMoveDelay{1400};

    HexCell* selected = nullptr;
    std::vector<HexCell> validMoves;
};