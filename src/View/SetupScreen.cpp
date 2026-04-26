#include "SetupScreen.hpp"

namespace {
    sf::Color playerColor(int i) {
        switch (i) {
            case 0:  return sf::Color(244, 244, 244);
            case 1:  return sf::Color(66,  96,  220);
            default: return sf::Color(214, 72,  72);
        }
    }
    const char* playerLabel(int i) {
        switch (i) {
            case 0:  return "Joueur 1 - Blancs";
            case 1:  return "Joueur 2 - Bleus";
            default: return "Joueur 3 - Rouges";
        }
    }
    const char* diffLabel(int d) {
        switch (d) {
            case 0:  return "Facile";
            case 1:  return "Normal";
            default: return "Difficile";
        }
    }

    void drawCentered(sf::RenderWindow& win, const sf::Font& font,
                      const sf::String& str, unsigned size, sf::Color color,
                      sf::FloatRect bounds, bool bold = false) {
        sf::Text t(font);
        t.setString(str);
        t.setCharacterSize(size);
        t.setFillColor(color);
        if (bold) t.setStyle(sf::Text::Bold);
        auto b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
        t.setPosition({bounds.position.x + bounds.size.x * 0.5f,
                       bounds.position.y + bounds.size.y * 0.5f});
        win.draw(t);
    }
}

SetupScreen::SetupScreen()
    : m_window(sf::VideoMode({800u, 590u}), "Yalta Chess - Configuration")
{
    m_fontLoaded =
        m_font.openFromFile("C:/Windows/Fonts/seguisym.ttf") ||
        m_font.openFromFile("C:/Windows/Fonts/segoeui.ttf")  ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");

    // defaults: P1 human, P2 AI depth 2, P3 AI depth 2
    m_config.players[0] = {false, 2};
    m_config.players[1] = {true,  2};
    m_config.players[2] = {true,  2};
}

GameConfig SetupScreen::run() {
    redraw();
    while (m_window.isOpen() && !m_started) {
        while (const auto event = m_window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                m_window.close();
                return m_config;
            }
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left)
                    handleClick(static_cast<float>(click->position.x),
                                static_cast<float>(click->position.y));
            }
        }
    }
    m_window.close();
    return m_config;
}

void SetupScreen::redraw() {
    constexpr float WIN_W   = 800.f;
    constexpr float WIN_H   = 590.f;
    constexpr float PANEL_W = 680.f;
    constexpr float ROW_H   = 112.f;
    constexpr float ROW_GAP = 10.f;
    constexpr float TITLE_H = 72.f;
    constexpr float PAD     = 22.f;
    constexpr float START_H = 58.f;
    const float PANEL_H = PAD + TITLE_H + 3.f * (ROW_H + ROW_GAP) - ROW_GAP + START_H + PAD * 1.5f;
    const float PANEL_X = (WIN_W - PANEL_W) * 0.5f;
    const float PANEL_Y = (WIN_H - PANEL_H) * 0.5f;

    m_window.clear(sf::Color(10, 10, 14));

    sf::RectangleShape panel({PANEL_W, PANEL_H});
    panel.setPosition({PANEL_X, PANEL_Y});
    panel.setFillColor(sf::Color(22, 24, 30, 242));
    panel.setOutlineColor(sf::Color(76, 84, 100));
    panel.setOutlineThickness(2.f);
    m_window.draw(panel);

    if (m_fontLoaded) {
        const sf::FloatRect titleR({PANEL_X, PANEL_Y + PAD * 0.3f}, {PANEL_W, TITLE_H * 0.55f});
        drawCentered(m_window, m_font, "Yalta Chess - 3 Joueurs", 27,
                     sf::Color(245, 245, 245), titleR, true);
        const sf::FloatRect subR({PANEL_X, PANEL_Y + PAD * 0.3f + 36.f}, {PANEL_W, 28.f});
        drawCentered(m_window, m_font, "Choisissez le type de chaque joueur", 14,
                     sf::Color(155, 162, 178), subR);
    }

    float rowY = PANEL_Y + PAD + TITLE_H;

    for (int i = 0; i < 3; ++i) {
        const float rowX = PANEL_X + PAD;
        const float rowW = PANEL_W - PAD * 2.f;

        sf::RectangleShape row({rowW, ROW_H});
        row.setPosition({rowX, rowY});
        row.setFillColor(sf::Color(34, 38, 47));
        row.setOutlineColor(sf::Color(58, 64, 78));
        row.setOutlineThickness(1.5f);
        m_window.draw(row);

        // Color dot
        sf::CircleShape dot(10.f);
        dot.setOrigin({10.f, 10.f});
        dot.setFillColor(playerColor(i));
        dot.setOutlineColor(sf::Color(245, 245, 245, 140));
        dot.setOutlineThickness(1.2f);
        dot.setPosition({rowX + 22.f, rowY + ROW_H * 0.28f});
        m_window.draw(dot);

        // Player name (top half)
        if (m_fontLoaded) {
            const sf::FloatRect nameR({rowX + 44.f, rowY}, {220.f, ROW_H * 0.52f});
            drawCentered(m_window, m_font, playerLabel(i), 18,
                         sf::Color(235, 238, 244), nameR, true);
        }

        // Human/AI toggle (top-right of row)
        constexpr float TOGGLE_W = 110.f;
        constexpr float TOGGLE_H = 42.f;
        const float toggleX = rowX + rowW - TOGGLE_W - 12.f;
        const float toggleY = rowY + (ROW_H * 0.52f - TOGGLE_H) * 0.5f;
        m_toggleBounds[i] = sf::FloatRect({toggleX, toggleY}, {TOGGLE_W, TOGGLE_H});

        sf::RectangleShape toggle({TOGGLE_W, TOGGLE_H});
        toggle.setPosition({toggleX, toggleY});
        toggle.setFillColor(m_config.players[i].isAI
                            ? sf::Color(90, 114, 228)
                            : sf::Color(72, 132, 92));
        toggle.setOutlineColor(sf::Color(245, 245, 245, 120));
        toggle.setOutlineThickness(1.5f);
        m_window.draw(toggle);
        if (m_fontLoaded)
            drawCentered(m_window, m_font,
                         m_config.players[i].isAI ? "IA" : "Humain", 17,
                         sf::Color::White, m_toggleBounds[i], true);

        // Difficulty buttons (bottom half of row)
        constexpr float DIFF_W   = 78.f;
        constexpr float DIFF_H   = 32.f;
        constexpr float DIFF_GAP = 6.f;
        const float diffStartX = rowX + rowW - 3.f * DIFF_W - 2.f * DIFF_GAP - 12.f;
        const float diffY = rowY + ROW_H * 0.52f + (ROW_H * 0.48f - DIFF_H) * 0.5f;

        for (int d = 0; d < 3; ++d) {
            const float dx = diffStartX + static_cast<float>(d) * (DIFF_W + DIFF_GAP);
            m_diffBounds[i][d] = sf::FloatRect({dx, diffY}, {DIFF_W, DIFF_H});

            const bool sel     = m_config.players[i].isAI && (m_config.players[i].aiDepth == d + 1);
            const bool enabled = m_config.players[i].isAI;

            sf::RectangleShape btn({DIFF_W, DIFF_H});
            btn.setPosition({dx, diffY});
            btn.setFillColor(sel ? sf::Color(224, 170, 58)
                             : (enabled ? sf::Color(50, 55, 68) : sf::Color(34, 37, 45)));
            btn.setOutlineColor(sel ? sf::Color(255, 239, 194)
                                : sf::Color(245, 245, 245, enabled ? 70u : 22u));
            btn.setOutlineThickness(1.2f);
            m_window.draw(btn);

            if (m_fontLoaded)
                drawCentered(m_window, m_font, diffLabel(d), 13,
                             sel      ? sf::Color(32, 24, 12)
                             : enabled ? sf::Color(210, 214, 224)
                                       : sf::Color(85, 90, 108),
                             m_diffBounds[i][d]);
        }

        rowY += ROW_H + ROW_GAP;
    }

    // Start button — disabled if all players are AI
    bool anyHuman = false;
    for (int i = 0; i < 3; ++i) if (!m_config.players[i].isAI) { anyHuman = true; break; }

    constexpr float START_W = 220.f;
    const float startX = PANEL_X + (PANEL_W - START_W) * 0.5f;
    const float startY = rowY + 10.f;
    m_startBounds = sf::FloatRect({startX, startY}, {START_W, START_H});

    sf::RectangleShape startBtn({START_W, START_H});
    startBtn.setPosition({startX, startY});
    startBtn.setFillColor(anyHuman ? sf::Color(224, 170, 58) : sf::Color(60, 62, 68));
    startBtn.setOutlineColor(anyHuman ? sf::Color(255, 239, 194) : sf::Color(90, 93, 100));
    startBtn.setOutlineThickness(2.f);
    m_window.draw(startBtn);
    if (m_fontLoaded)
        drawCentered(m_window, m_font, "Demarrer la partie", 21,
                     anyHuman ? sf::Color(32, 24, 12) : sf::Color(110, 112, 118),
                     m_startBounds, true);

    m_window.display();
}

void SetupScreen::handleClick(float x, float y) {
    const sf::Vector2f mouse{x, y};

    for (int i = 0; i < 3; ++i) {
        if (m_toggleBounds[i].contains(mouse)) {
            m_config.players[i].isAI = !m_config.players[i].isAI;
            redraw();
            return;
        }
    }

    for (int i = 0; i < 3; ++i) {
        for (int d = 0; d < 3; ++d) {
            if (m_config.players[i].isAI && m_diffBounds[i][d].contains(mouse)) {
                m_config.players[i].aiDepth = d + 1;
                redraw();
                return;
            }
        }
    }

    bool anyHuman = false;
    for (int i = 0; i < 3; ++i) if (!m_config.players[i].isAI) { anyHuman = true; break; }

    if (anyHuman && m_startBounds.contains(mouse))
        m_started = true;
}
