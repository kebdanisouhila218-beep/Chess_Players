#pragma once
#include <SFML/Graphics.hpp>
#include "../Model/GameConfig.hpp"

class SetupScreen {
public:
    SetupScreen();
    GameConfig run();   // blocking — returns when user clicks Start or closes window
    bool wasStarted() const { return m_started; }

private:
    void redraw();
    void handleClick(float x, float y);

    sf::RenderWindow m_window;
    sf::Font         m_font;
    bool             m_fontLoaded = false;
    GameConfig       m_config;
    bool             m_started = false;

    sf::FloatRect m_toggleBounds[3];
    sf::FloatRect m_diffBounds[3][3]; // [player][level 0-2]
    sf::FloatRect m_startBounds;
};
