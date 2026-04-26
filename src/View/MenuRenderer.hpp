#pragma once
#include <SFML/Graphics.hpp>
#include <array>

class MenuRenderer {
public:
    explicit MenuRenderer(sf::RenderWindow& window);

    void draw(const std::array<bool, 3>& isAI, int aiDepth) const;
    std::array<sf::FloatRect, 3> getToggleBounds() const;
    sf::FloatRect getStartButtonBounds() const;
    std::array<sf::FloatRect, 3> getDifficultyBounds() const;

private:
    struct MenuLayout {
        sf::FloatRect panelBounds;
        sf::FloatRect titleBounds;
        std::array<sf::FloatRect, 3> rowBounds;
        std::array<sf::FloatRect, 3> toggleBounds;
        sf::FloatRect difficultyRowBounds;
        std::array<sf::FloatRect, 3> difficultyBounds;
        sf::FloatRect startBounds;
    };

    MenuLayout computeLayout() const;
    void drawCenteredText(const sf::String& text, unsigned int size, sf::Color color,
                          const sf::FloatRect& bounds, bool bold = false) const;

    sf::RenderWindow& window;
    sf::Font font;
    bool fontLoaded = false;
};
