#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <vector>
#include "../Model/GameState.hpp"
#include "../Model/IObserver.hpp"

class Renderer : public IObserver {
public:
    Renderer(sf::RenderWindow& window);

    void onStateChanged() override;
    void draw(const GameState& state);
    void highlight(const HexCell& c);

    void setHighlights(const std::vector<HexCell>& cells);
    void clearHighlights();
    void setStatusMessage(const std::string& message);

    sf::Vector2f cellToPixel(const Board& board, const HexCell& c) const;
    std::optional<HexCell> pickCell(const Board& board, sf::Vector2f px) const;

    void setCurrentState(const GameState* state) { currentState = state; }

private:
    void initBoardGeometry(const Board& board);
    void drawBoard(const GameState& state);
    void drawPieces(const GameState& state);
    sf::ConvexShape createTile(const std::array<sf::Vector2f, 4>& points, sf::Color color) const;
    void drawHUD(const GameState& state);        // <-- nouveau

    sf::RenderWindow& window;
    const GameState*  currentState = nullptr;
    float             tileRadius   = 18.f;

    sf::Font m_font;
    bool     m_fontLoaded = false;
    std::vector<HexCell> m_highlights;
    std::vector<sf::ConvexShape> m_cellShapes;
    std::vector<sf::Vector2f> m_cellCenters;
    std::vector<sf::Color> m_baseColors;
    std::string m_statusMessage;
    bool m_geometryReady = false;
    sf::Vector2u m_lastWindowSize{0u, 0u};
};