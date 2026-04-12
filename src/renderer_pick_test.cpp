#include "Model/GameState.hpp"
#include "Model/PieceFactory.hpp"
#include "View/Renderer.hpp"

#include <iostream>
#include <sstream>
#include <vector>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 850}), "RendererPickTest", sf::Style::Titlebar);
    window.setVisible(false);

    GameState state;
    PieceFactory factory;
    factory.initBoard(state.getBoard(), Player::PLAYER1);
    factory.initBoard(state.getBoard(), Player::PLAYER2);
    factory.initBoard(state.getBoard(), Player::PLAYER3);

    Renderer renderer(window);
    renderer.setCurrentState(&state);
    renderer.draw(state);

    int checked = 0;
    int failures = 0;
    std::vector<std::string> mismatchSamples;

    for (const HexCell& cell : state.getBoard().allValidCells()) {
        const sf::Vector2f center = renderer.cellToPixel(state.getBoard(), cell);
        const std::optional<HexCell> picked = renderer.pickCell(state.getBoard(), center);
        ++checked;

        if (!picked.has_value() || !(*picked == cell)) {
            ++failures;
            if (mismatchSamples.size() < 12) {
                std::ostringstream out;
                out << "center(" << cell.q << ',' << cell.r << ") -> ";
                if (picked.has_value()) {
                    out << '(' << picked->q << ',' << picked->r << ')';
                } else {
                    out << "none";
                }
                mismatchSamples.push_back(out.str());
            }
        }
    }

    std::cout << "Renderer click picking test\n";
    std::cout << "checked=" << checked << " failures=" << failures << "\n";
    for (const std::string& sample : mismatchSamples) {
        std::cout << sample << "\n";
    }

    return failures == 0 ? 0 : 1;
}
