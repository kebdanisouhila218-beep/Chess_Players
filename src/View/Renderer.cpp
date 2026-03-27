#include "Renderer.hpp"
#include <cmath>

Renderer::Renderer(sf::RenderWindow& window)
    : window(window) {}

void Renderer::onStateChanged() {
    if (currentState)
        draw(*currentState);
}

sf::Vector2f Renderer::hexToPixel(const HexCell& c) const {
    float x = tileSize * (3.f / 2.f * c.q);
    float y = tileSize * (std::sqrt(3.f) / 2.f * c.q + std::sqrt(3.f) * c.r);
    // Centre de la fenêtre
    x += 400.f;
    y += 400.f;
    return {x, y};
}

HexCell Renderer::pixelToHex(sf::Vector2f px) const {
    px.x -= 400.f;
    px.y -= 400.f;
    float q = (2.f / 3.f * px.x) / tileSize;
    float r = (-1.f / 3.f * px.x + std::sqrt(3.f) / 3.f * px.y) / tileSize;
    // Arrondi hex
    int rq = (int)std::round(q);
    int rr = (int)std::round(r);
    int rs = (int)std::round(-q - r);
    if (rq + rr + rs != 0) {
        float dq = std::abs(rq - q);
        float dr = std::abs(rr - r);
        float ds = std::abs(rs - (-q - r));
        if (dq > dr && dq > ds) rq = -rr - rs;
        else if (dr > ds)       rr = -rq - rs;
    }
    return {rq, rr};
}

void Renderer::drawHex(const HexCell& c, sf::Color color) {
    sf::CircleShape hex(tileSize - 2.f, 6);
    hex.setFillColor(color);
    hex.setOutlineColor(sf::Color(80, 80, 80));
    hex.setOutlineThickness(1.f);
    hex.setOrigin({tileSize, tileSize});
    sf::Vector2f pos = hexToPixel(c);
    hex.setPosition(pos);
    window.draw(hex);
}

void Renderer::drawBoard(const GameState& state) {
    const Board& board = state.getBoard();
    for (const HexCell& c : board.allValidCells()) {
        sf::Color color = sf::Color(200, 180, 140);
        if ((c.q + c.r + c.s() + 99) % 3 == 0)
            color = sf::Color(160, 120, 80);
        else if ((c.q + c.r + c.s() + 99) % 3 == 1)
            color = sf::Color(240, 220, 180);
        drawHex(c, color);
    }
}

void Renderer::drawPieces(const GameState& state) {
    const Board& board = state.getBoard();
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;

        sf::CircleShape piece(tileSize * 0.4f);
        if (p->getOwner() == Player::PLAYER1)
            piece.setFillColor(sf::Color(255, 255, 255));
        else if (p->getOwner() == Player::PLAYER2)
            piece.setFillColor(sf::Color(50, 50, 200));
        else
            piece.setFillColor(sf::Color(200, 50, 50));

        piece.setOutlineColor(sf::Color::Black);
        piece.setOutlineThickness(1.f);
        piece.setOrigin({tileSize * 0.4f, tileSize * 0.4f});
        piece.setPosition(hexToPixel(c));
        window.draw(piece);
    }
}

void Renderer::highlight(const HexCell& c) {
    drawHex(c, sf::Color(100, 220, 100));
}

void Renderer::draw(const GameState& state) {
    window.clear(sf::Color(30, 30, 30));
    drawBoard(state);
    drawPieces(state);
    window.display();
}