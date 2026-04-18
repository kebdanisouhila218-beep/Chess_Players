#include "MenuRenderer.hpp"
#include <string>

namespace {
    std::string playerLabel(std::size_t index) {
        switch (index) {
            case 0: return "Joueur 1 - Blancs";
            case 1: return "Joueur 2 - Bleus";
            case 2: return "Joueur 3 - Rouges";
            default: return "Joueur";
        }
    }

    sf::Color playerColor(std::size_t index) {
        switch (index) {
            case 0: return sf::Color(244, 244, 244);
            case 1: return sf::Color(66, 96, 220);
            case 2: return sf::Color(214, 72, 72);
            default: return sf::Color::White;
        }
    }
}

MenuRenderer::MenuRenderer(sf::RenderWindow& window)
    : window(window) {
    fontLoaded =
        font.openFromFile("C:/Windows/Fonts/seguisym.ttf") ||
        font.openFromFile("C:/Windows/Fonts/segoeui.ttf") ||
        font.openFromFile("C:/Windows/Fonts/arial.ttf");
}

MenuRenderer::MenuLayout MenuRenderer::computeLayout() const {
    const sf::Vector2u winSize = window.getSize();
    const float width = static_cast<float>(winSize.x);
    const float height = static_cast<float>(winSize.y);

    const float panelWidth = std::min(width * 0.78f, 620.f);
    const float rowHeight = 78.f;
    const float rowGap = 18.f;
    const float titleHeight = 70.f;
    const float startHeight = 62.f;
    const float verticalPadding = 28.f;
    const float panelHeight = titleHeight + verticalPadding * 2.f + rowHeight * 3.f + rowGap * 2.f + startHeight + 30.f;
    const float panelX = (width - panelWidth) * 0.5f;
    const float panelY = (height - panelHeight) * 0.5f;

    MenuLayout layout;
    layout.panelBounds = sf::FloatRect({panelX, panelY}, {panelWidth, panelHeight});
    layout.titleBounds = sf::FloatRect({panelX + 24.f, panelY + 14.f}, {panelWidth - 48.f, titleHeight});

    float currentY = panelY + titleHeight + verticalPadding;
    for (std::size_t i = 0; i < 3; ++i) {
        layout.rowBounds[i] = sf::FloatRect({panelX + 26.f, currentY}, {panelWidth - 52.f, rowHeight});
        layout.toggleBounds[i] = sf::FloatRect({panelX + panelWidth - 182.f, currentY + 14.f}, {132.f, 50.f});
        currentY += rowHeight + rowGap;
    }

    layout.startBounds = sf::FloatRect({panelX + (panelWidth - 220.f) * 0.5f, panelY + panelHeight - startHeight - 22.f}, {220.f, startHeight});
    return layout;
}

std::array<sf::FloatRect, 3> MenuRenderer::getToggleBounds() const {
    return computeLayout().toggleBounds;
}

sf::FloatRect MenuRenderer::getStartButtonBounds() const {
    return computeLayout().startBounds;
}

void MenuRenderer::drawCenteredText(const sf::String& text, unsigned int size, sf::Color color,
                                    const sf::FloatRect& bounds, bool bold) const {
    if (!fontLoaded) {
        return;
    }

    sf::Text label(font);
    label.setString(text);
    label.setCharacterSize(size);
    label.setFillColor(color);
    if (bold) {
        label.setStyle(sf::Text::Bold);
    }

    const auto local = label.getLocalBounds();
    label.setOrigin({local.position.x + local.size.x * 0.5f, local.position.y + local.size.y * 0.5f});
    label.setPosition({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
    window.draw(label);
}

void MenuRenderer::draw(const std::array<bool, 3>& isAI) const {
    const MenuLayout layout = computeLayout();

    window.clear(sf::Color(10, 10, 14));

    sf::RectangleShape background({layout.panelBounds.size.x, layout.panelBounds.size.y});
    background.setPosition({layout.panelBounds.position.x, layout.panelBounds.position.y});
    background.setFillColor(sf::Color(22, 24, 30, 242));
    background.setOutlineColor(sf::Color(76, 84, 100));
    background.setOutlineThickness(2.f);
    window.draw(background);

    drawCenteredText("Configuration de la partie", 30, sf::Color(245, 245, 245), layout.titleBounds, true);

    sf::FloatRect subtitleBounds = layout.titleBounds;
    subtitleBounds.position.y += 34.f;
    drawCenteredText("Choisissez pour chaque joueur : Humain ou IA", 16, sf::Color(185, 190, 200), subtitleBounds);

    for (std::size_t i = 0; i < 3; ++i) {
        sf::RectangleShape row({layout.rowBounds[i].size.x, layout.rowBounds[i].size.y});
        row.setPosition({layout.rowBounds[i].position.x, layout.rowBounds[i].position.y});
        row.setFillColor(sf::Color(34, 38, 47));
        row.setOutlineColor(sf::Color(58, 64, 78));
        row.setOutlineThickness(1.5f);
        window.draw(row);

        sf::CircleShape dot(9.f);
        dot.setOrigin({9.f, 9.f});
        dot.setPosition({layout.rowBounds[i].position.x + 24.f, layout.rowBounds[i].position.y + layout.rowBounds[i].size.y * 0.5f});
        dot.setFillColor(playerColor(i));
        dot.setOutlineColor(sf::Color(245, 245, 245, 140));
        dot.setOutlineThickness(1.2f);
        window.draw(dot);

        sf::FloatRect labelBounds = layout.rowBounds[i];
        labelBounds.position.x += 44.f;
        labelBounds.size.x -= 200.f;
        drawCenteredText(playerLabel(i), 21, sf::Color(235, 238, 244), labelBounds, true);

        sf::RectangleShape toggle({layout.toggleBounds[i].size.x, layout.toggleBounds[i].size.y});
        toggle.setPosition({layout.toggleBounds[i].position.x, layout.toggleBounds[i].position.y});
        toggle.setFillColor(isAI[i] ? sf::Color(90, 114, 228) : sf::Color(72, 132, 92));
        toggle.setOutlineColor(sf::Color(245, 245, 245, 120));
        toggle.setOutlineThickness(1.5f);
        window.draw(toggle);

        drawCenteredText(isAI[i] ? "IA" : "Humain", 20, sf::Color::White, layout.toggleBounds[i], true);
    }

    sf::RectangleShape start({layout.startBounds.size.x, layout.startBounds.size.y});
    start.setPosition({layout.startBounds.position.x, layout.startBounds.position.y});
    start.setFillColor(sf::Color(224, 170, 58));
    start.setOutlineColor(sf::Color(255, 239, 194));
    start.setOutlineThickness(2.f);
    window.draw(start);

    drawCenteredText("Demarrer", 24, sf::Color(32, 24, 12), layout.startBounds, true);

    window.display();
}
