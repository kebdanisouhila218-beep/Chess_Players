#pragma once
#include <SFML/Graphics.hpp>
#include "../Model/GameState.hpp"
#include "../Model/IObserver.hpp"

class Renderer : public IObserver {
public:
    Renderer(sf::RenderWindow& window);

    void onStateChanged() override;
    void draw(const GameState& state);
    void highlight(const HexCell& c);

    sf::Vector2f hexToPixel(const HexCell& c) const;
    HexCell      pixelToHex(sf::Vector2f px) const;

    void setCurrentState(const GameState* state) { currentState = state; }

private:
    void drawBoard(const GameState& state);
    void drawPieces(const GameState& state);
    void drawHex(const HexCell& c, sf::Color color);

    sf::RenderWindow& window;
    const GameState*  currentState = nullptr;
    float             tileSize     = 40.f;
};