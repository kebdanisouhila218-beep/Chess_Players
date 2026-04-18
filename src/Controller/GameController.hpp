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
    GameController();
    void run();

private:
    void handleEvents();
    void handleClick(int x, int y);
    void handleMenuClick(int x, int y);
    void tryAIMove();
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