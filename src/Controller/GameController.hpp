#pragma once
#include <SFML/Graphics.hpp>
#include "../Model/GameState.hpp"
#include "../Model/PieceFactory.hpp"
#include "../View/Renderer.hpp"

class GameController {
public:
    GameController();
    void run();

private:
    void handleEvents();
    void handleClick(int x, int y);

    sf::RenderWindow window;
    GameState        state;
    Renderer         renderer;
    PieceFactory     factory;

    HexCell* selected = nullptr;
    std::vector<HexCell> validMoves;
};