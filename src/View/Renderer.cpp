#include "Renderer.hpp"
#include <array>
#include <cmath>
#include <cstdint>

namespace {
    bool inSextantIntervals(int x, int y, int& sextant) {
        static constexpr std::array<std::array<int, 4>, 6> intervals = {
            std::array<int, 4>{0, 4, 0, 4},
            std::array<int, 4>{0, 4, 4, 8},
            std::array<int, 4>{8, 12, 4, 8},
            std::array<int, 4>{8, 12, 8, 12},
            std::array<int, 4>{4, 8, 8, 12},
            std::array<int, 4>{4, 8, 0, 4}
        };

        for (int i = 0; i < 6; ++i) {
            const auto& it = intervals[i];
            if (x >= it[0] && x < it[1] && y >= it[2] && y < it[3]) {
                sextant = i;
                return true;
            }
        }
        return false;
    }

    sf::Vector2f addVec(const sf::Vector2f& a, const sf::Vector2f& b) {
        return {a.x + b.x, a.y + b.y};
    }

    sf::Vector2f subVec(const sf::Vector2f& a, const sf::Vector2f& b) {
        return {a.x - b.x, a.y - b.y};
    }

    sf::Vector2f mulVec(const sf::Vector2f& a, float k) {
        return {a.x * k, a.y * k};
    }

    sf::String chessGlyph(PieceType t, bool whiteSet) {
        if (whiteSet) {
            switch (t) {
                case PieceType::KING:   return sf::String(U"\u2654");
                case PieceType::QUEEN:  return sf::String(U"\u2655");
                case PieceType::ROOK:   return sf::String(U"\u2656");
                case PieceType::BISHOP: return sf::String(U"\u2657");
                case PieceType::KNIGHT: return sf::String(U"\u2658");
                case PieceType::PAWN:   return sf::String(U"\u2659");
                default: return sf::String("?");
            }
        }

        switch (t) {
            case PieceType::KING:   return sf::String(U"\u265A");
            case PieceType::QUEEN:  return sf::String(U"\u265B");
            case PieceType::ROOK:   return sf::String(U"\u265C");
            case PieceType::BISHOP: return sf::String(U"\u265D");
            case PieceType::KNIGHT: return sf::String(U"\u265E");
            case PieceType::PAWN:   return sf::String(U"\u265F");
            default: return sf::String("?");
        }
    }

    float cross2d(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& p) {
        return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    }

    bool pointInConvexQuad(const sf::ConvexShape& shape, sf::Vector2f point) {
        const std::size_t count = shape.getPointCount();
        if (count < 3) {
            return false;
        }

        float sign = 0.f;
        for (std::size_t i = 0; i < count; ++i) {
            const sf::Vector2f a = shape.getPoint(i);
            const sf::Vector2f b = shape.getPoint((i + 1) % count);
            const float cross = cross2d(a, b, point);
            if (std::abs(cross) < 0.01f) {
                continue;
            }

            if (sign == 0.f) {
                sign = cross;
                continue;
            }

            if ((sign < 0.f && cross > 0.f) || (sign > 0.f && cross < 0.f)) {
                return false;
            }
        }

        return true;
    }

    sf::Vector2f midpoint(const sf::Vector2f& a, const sf::Vector2f& b) {
        return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
    }
}

Renderer::Renderer(sf::RenderWindow& window)
    : window(window) {
    m_fontLoaded =
        m_font.openFromFile("C:/Windows/Fonts/seguisym.ttf") ||
        m_font.openFromFile("C:/Windows/Fonts/segoeui.ttf")  ||
        m_font.openFromFile("C:/Windows/Fonts/arial.ttf");
    m_highlights.clear();
    m_selectedCell.reset();
}

void Renderer::onStateChanged() {
    if (currentState)
        draw(*currentState);
}

sf::Vector2f Renderer::cellToPixel(const Board& board, const HexCell& c) const {
    const int id = board.getId(c);
    if (id >= 0 && id < static_cast<int>(m_cellCenters.size())) {
        return m_cellCenters[id];
    }
    ScreenCoord pos = board.getScreenPosition(c);
    return {pos.x, pos.y};
}

std::optional<HexCell> Renderer::pickCell(const Board& board, sf::Vector2f px) const {
    for (std::size_t i = 0; i < m_cellShapes.size(); ++i) {
        if (!m_cellShapes[i].getGlobalBounds().contains(px)) {
            continue;
        }
        if (pointInConvexQuad(m_cellShapes[i], px)) {
            return board.getCellById(static_cast<int>(i));
        }
    }

    const float threshold = 26.f;
    float minDist = threshold;
    int clickedId = -1;

    for (size_t i = 0; i < m_cellCenters.size(); ++i) {
        sf::Vector2f diff = px - m_cellCenters[i];
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist < minDist) {
            minDist = dist;
            clickedId = static_cast<int>(i);
        }
    }

    if (clickedId < 0) {
        return std::nullopt;
    }
    return board.getCellById(clickedId);
}

sf::ConvexShape Renderer::createTile(const std::array<sf::Vector2f, 4>& points, sf::Color color) const {
    sf::ConvexShape tile(4);
    tile.setPoint(0, points[0]);
    tile.setPoint(1, points[1]);
    tile.setPoint(2, points[2]);
    tile.setPoint(3, points[3]);
    tile.setFillColor(color);
    tile.setOutlineColor(sf::Color(30, 20, 15, 120));
    tile.setOutlineThickness(0.7f);
    return tile;
}

void Renderer::initBoardGeometry(const Board& board) {
    const sf::Vector2u winSize = window.getSize();
    if (m_geometryReady && winSize == m_lastWindowSize) {
        return;
    }

    const float boardSize = std::min(static_cast<float>(winSize.x) * 0.40f,
                                     (static_cast<float>(winSize.y) - 46.f) * 0.54f);
    const float side = boardSize * 0.5f;
    const float height = std::sqrt(boardSize * boardSize - side * side);
    const float c30 = std::cos(30.f * 3.1415926535f / 180.f);
    const float c60 = std::cos(60.f * 3.1415926535f / 180.f);

    const std::array<sf::Vector2f, 6> v123 = {
        sf::Vector2f(-boardSize * c60, -height),
        sf::Vector2f( boardSize * c60, -height),
        sf::Vector2f( boardSize, 0.f),
        sf::Vector2f( boardSize * c60, height),
        sf::Vector2f(-boardSize * c60, height),
        sf::Vector2f(-boardSize, 0.f)
    };

    const std::array<sf::Vector2f, 6> vabc = {
        sf::Vector2f(-height * c30, -height * 0.5f),
        sf::Vector2f(0.f, -height),
        sf::Vector2f(height * c30, -height * 0.5f),
        sf::Vector2f(height * c30, height * 0.5f),
        sf::Vector2f(0.f, height),
        sf::Vector2f(-height * c30, height * 0.5f)
    };

    const float hudHeight = 46.f;
    const sf::Vector2f boardCenter{
        static_cast<float>(winSize.x) * 0.5f,
        (static_cast<float>(winSize.y) - hudHeight) * 0.5f - 2.f
    };

    m_cellShapes.clear();
    m_cellCenters.clear();
    m_baseColors.clear();
    m_cellShapes.reserve(96);
    m_cellCenters.reserve(96);
    m_baseColors.reserve(96);

    for (int id = 0; id < Board::CELL_COUNT; ++id) {
        HexCell cell = board.getCellById(id);
        const int x = cell.q;
        const int y = cell.r;
        int sextant = -1;
        inSextantIntervals(x, y, sextant);

        const float ratioX1 = static_cast<float>(x % 4) / 4.f;
        const float ratioY1 = static_cast<float>(y % 4) / 4.f;
        const float ratioX2 = static_cast<float>(x % 4 + 1) / 4.f;
        const float ratioY2 = static_cast<float>(y % 4 + 1) / 4.f;
        const float midRatioX = (ratioX1 + ratioX2) * 0.5f;
        const float midRatioY = (ratioY1 + ratioY2) * 0.5f;

        const sf::Vector2f s1 = mulVec(v123[sextant], 0.5f);
        const sf::Vector2f s2 = mulVec(v123[(sextant + 2) % 6], 0.5f);
        const sf::Vector2f corner = addVec(boardCenter, v123[(sextant + 4) % 6]);

        const sf::Vector2f u1 = addVec(subVec(mulVec(vabc[(sextant + 1) % 6], ratioY1), mulVec(s1, ratioY1)), s2);
        const sf::Vector2f u2 = addVec(subVec(mulVec(vabc[(sextant + 1) % 6], ratioY2), mulVec(s1, ratioY2)), s2);
        const sf::Vector2f midU = addVec(subVec(mulVec(vabc[(sextant + 1) % 6], midRatioY), mulVec(s1, midRatioY)), s2);

        const sf::Vector2f p1 = addVec(corner, addVec(mulVec(s1, ratioY1), mulVec(u1, ratioX1)));
        const sf::Vector2f p2 = addVec(corner, addVec(mulVec(s1, ratioY1), mulVec(u1, ratioX2)));
        const sf::Vector2f p3 = addVec(corner, addVec(mulVec(s1, ratioY2), mulVec(u2, ratioX2)));
        const sf::Vector2f p4 = addVec(corner, addVec(mulVec(s1, ratioY2), mulVec(u2, ratioX1)));
        const sf::Vector2f center = addVec(corner, addVec(mulVec(s1, midRatioY), mulVec(midU, midRatioX)));

        const sf::Color dark(18, 18, 18);
        const sf::Color light(226, 209, 169);
        const sf::Color fillColor = ((x + y + sextant) % 2 == 0) ? dark : light;
        m_baseColors.push_back(fillColor);
        m_cellCenters.push_back(center);
        m_cellShapes.push_back(createTile({p1, p2, p3, p4}, fillColor));
    }

    m_geometryReady = true;
    m_lastWindowSize = winSize;
}

void Renderer::drawBoard(const GameState& state) {
    const Board& board = state.getBoard();
    initBoardGeometry(board);

    for (size_t i = 0; i < m_cellShapes.size(); ++i) {
        m_cellShapes[i].setFillColor(m_baseColors[i]);
        m_cellShapes[i].setOutlineColor(sf::Color(247, 243, 228));
        m_cellShapes[i].setOutlineThickness(1.2f);
    }

    for (const HexCell& c : m_highlights) {
        const int id = board.getId(c);
        if (id < 0 || id >= static_cast<int>(m_cellShapes.size())) continue;

        sf::Color blended = m_baseColors[id];
        blended.r = static_cast<std::uint8_t>(std::min(255, blended.r + 20));
        blended.g = static_cast<std::uint8_t>(std::min(255, blended.g + 60));
        blended.b = static_cast<std::uint8_t>(std::min(255, blended.b + 28));
        m_cellShapes[id].setFillColor(blended);
        m_cellShapes[id].setOutlineColor(sf::Color(144, 238, 144));
        m_cellShapes[id].setOutlineThickness(2.8f);
    }

    if (m_selectedCell.has_value()) {
        const int id = board.getId(*m_selectedCell);
        if (id >= 0 && id < static_cast<int>(m_cellShapes.size())) {
            const auto elapsed = std::chrono::steady_clock::now() - m_selectionPulseStart;
            const float t = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.f;
            const float pulse = 0.5f + 0.5f * std::sin(t * 6.283185307f * 1.2f);
            sf::Color outline(
                static_cast<std::uint8_t>(220 + 30 * pulse),
                static_cast<std::uint8_t>(180 + 50 * pulse),
                80,
                255);
            m_cellShapes[id].setOutlineColor(outline);
            m_cellShapes[id].setOutlineThickness(3.2f + pulse * 1.6f);
        }
    }

    for (const sf::ConvexShape& tile : m_cellShapes) {
        window.draw(tile);
    }

    drawSeams(board);
}

void Renderer::drawSeams(const Board& board) {
    auto drawSeam = [&](const HexCell& from, const HexCell& to, float offsetScale) {
        const int fromId = board.getId(from);
        const int toId = board.getId(to);
        if (fromId < 0 || toId < 0 ||
            fromId >= static_cast<int>(m_cellShapes.size()) || toId >= static_cast<int>(m_cellShapes.size())) {
            return;
        }

        const sf::ConvexShape& a = m_cellShapes[fromId];
        const sf::ConvexShape& b = m_cellShapes[toId];
        const sf::Vector2f ca = m_cellCenters[fromId];
        const sf::Vector2f cb = m_cellCenters[toId];
        const sf::Vector2f dir = cb - ca;
        const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.001f) {
            return;
        }

        const sf::Vector2f normal{-dir.y / len, dir.x / len};
        sf::Vector2f bestA = midpoint(a.getPoint(0), a.getPoint(1));
        sf::Vector2f bestB = midpoint(b.getPoint(0), b.getPoint(1));
        float bestProjA = -1e9f;
        float bestProjB = -1e9f;

        for (std::size_t i = 0; i < a.getPointCount(); ++i) {
            const sf::Vector2f m = midpoint(a.getPoint(i), a.getPoint((i + 1) % a.getPointCount()));
            const sf::Vector2f rel = m - ca;
            const float proj = rel.x * dir.x + rel.y * dir.y;
            if (proj > bestProjA) {
                bestProjA = proj;
                bestA = m;
            }
        }

        for (std::size_t i = 0; i < b.getPointCount(); ++i) {
            const sf::Vector2f m = midpoint(b.getPoint(i), b.getPoint((i + 1) % b.getPointCount()));
            const sf::Vector2f rel = m - cb;
            const float proj = rel.x * dir.x + rel.y * dir.y;
            if (proj < bestProjB) {
                bestProjB = proj;
                bestB = m;
            }
        }

        sf::VertexArray seam(sf::PrimitiveType::Lines, 2);
        seam[0].position = bestA + normal * offsetScale;
        seam[1].position = bestB + normal * offsetScale;
        seam[0].color = sf::Color(255, 215, 140, 145);
        seam[1].color = sf::Color(255, 215, 140, 145);
        window.draw(seam);
    };

    for (int k = 0; k < 4; ++k) {
        drawSeam({7, k}, {4 + k, 11}, -2.0f);
        drawSeam({k, 3}, {3, 4 + k}, 0.0f);
        drawSeam({8 + k, 7}, {11, 8 + k}, 2.0f);
    }
}

void Renderer::drawPieces(const GameState& state) {
    const Board& board = state.getBoard();
    initBoardGeometry(board);
    for (const HexCell& c : board.allValidCells()) {
        Piece* p = board.getPiece(c);
        if (!p) continue;

        sf::CircleShape token(12.5f);

        sf::Color tokenColor;
        if (p->getOwner() == Player::PLAYER1)
            tokenColor = sf::Color(244, 244, 244);
        else if (p->getOwner() == Player::PLAYER2)
            tokenColor = sf::Color(66, 96, 220);
        else
            tokenColor = sf::Color(214, 72, 72);
        token.setFillColor(tokenColor);

        token.setOutlineColor(sf::Color(15, 15, 15, 180));
        token.setOutlineThickness(1.6f);
        token.setOrigin({12.5f, 12.5f});
        token.setPosition(cellToPixel(board, c) + sf::Vector2f(0.f, -6.f));
        window.draw(token);

        if (m_fontLoaded) {
            const bool whiteSet = (p->getOwner() == Player::PLAYER1);
            sf::Text text(m_font);
            text.setString(chessGlyph(p->getType(), whiteSet));
            text.setCharacterSize(19);

            text.setFillColor(p->getOwner() == Player::PLAYER1
                                  ? sf::Color(28, 28, 28)
                                  : sf::Color(245, 245, 245));
            auto bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x * 0.5f,
                            bounds.position.y + bounds.size.y * 0.58f});
            text.setPosition(token.getPosition());
            window.draw(text);
        }
    }
}

void Renderer::drawHUD(const GameState& state) {
    if (!m_fontLoaded) return;

    sf::Vector2u winSize = window.getSize();
    float winW = static_cast<float>(winSize.x);
    float winH = static_cast<float>(winSize.y);
    float hudH = 46.f;
    float hudY = winH - hudH;

    // Fond du HUD
    sf::RectangleShape bg({winW, hudH});
    bg.setPosition({0.f, hudY});
    bg.setFillColor(sf::Color(20, 20, 20, 220));
    window.draw(bg);

    // Ligne de separation
    sf::RectangleShape sep({winW, 2.f});
    sep.setPosition({0.f, hudY});
    sep.setFillColor(sf::Color(60, 60, 60));
    window.draw(sep);

    // Couleur et label selon le joueur courant
    sf::Color playerColor;
    std::string playerName;
    std::string playerLabel;

    switch (state.getCurrentPlayer()) {
        case Player::PLAYER1:
            playerColor = sf::Color(244, 244, 244);
            playerName  = "Joueur 1";
            playerLabel = "Blancs";
            break;
        case Player::PLAYER2:
            playerColor = sf::Color(66, 96, 220);
            playerName  = "Joueur 2";
            playerLabel = "Bleus";
            break;
        case Player::PLAYER3:
            playerColor = sf::Color(214, 72, 72);
            playerName  = "Joueur 3";
            playerLabel = "Rouges";
            break;
        default:
            return;
    }

    // Bande coloree a gauche
    sf::RectangleShape stripe({6.f, hudH});
    stripe.setPosition({0.f, hudY});
    stripe.setFillColor(playerColor);
    window.draw(stripe);

    // Cercle colore (pastille)
    float cy = hudY + hudH / 2.f;
    sf::CircleShape dot(12.f);
    dot.setFillColor(playerColor);
    dot.setOutlineColor(
        playerColor == sf::Color(244, 244, 244)
            ? sf::Color(255, 255, 255, 200)
            : sf::Color(0, 0, 0, 100));
    dot.setOutlineThickness(1.5f);
    dot.setOrigin({12.f, 12.f});
    dot.setPosition({30.f, cy});
    window.draw(dot);

    // Texte principal : "A jouer :"
    sf::Text labelText(m_font);
    labelText.setString("A jouer :");
    labelText.setCharacterSize(14);
    labelText.setFillColor(sf::Color(160, 160, 160));
    {
        auto b = labelText.getLocalBounds();
        labelText.setOrigin({b.position.x, b.position.y + b.size.y * 0.5f});
    }
    labelText.setPosition({54.f, cy});
    window.draw(labelText);

    // Texte joueur : "Joueur 1 — Blancs"
    sf::Text nameText(m_font);
    nameText.setString(playerName + " - " + playerLabel);
    nameText.setCharacterSize(18);
    nameText.setFillColor(playerColor);
    nameText.setStyle(sf::Text::Bold);
    {
        auto b = nameText.getLocalBounds();
        nameText.setOrigin({b.position.x, b.position.y + b.size.y * 0.5f});
    }
    nameText.setPosition({140.f, cy});
    window.draw(nameText);

    if (!m_statusMessage.empty()) {
        sf::Text statusText(m_font);
        statusText.setString(m_statusMessage);
        statusText.setCharacterSize(13);
        statusText.setFillColor(sf::Color(205, 205, 205));
        auto b = statusText.getLocalBounds();
        statusText.setOrigin({b.position.x + b.size.x, b.position.y + b.size.y * 0.5f});
        statusText.setPosition({winW - 92.f, cy});
        window.draw(statusText);
    }

    // Indicateur visuel des 3 joueurs (petits cercles a droite)
    float dotX = winW - 20.f;
    struct PlayerDot { Player p; sf::Color c; };
    // Affiches dans l'ordre inverse pour que P1 soit a droite
    for (auto& pd : {
            PlayerDot{Player::PLAYER3, sf::Color(214, 72, 72)},
            PlayerDot{Player::PLAYER2, sf::Color(66, 96, 220)},
            PlayerDot{Player::PLAYER1, sf::Color(244, 244, 244)}
    }) {
        float r = (pd.p == state.getCurrentPlayer()) ? 10.f : 6.f;
        sf::CircleShape d(r);
        d.setFillColor(pd.c);
        d.setOutlineColor(
            pd.p == state.getCurrentPlayer()
                ? sf::Color(255, 255, 255, 200)
                : sf::Color(0, 0, 0, 100));
        d.setOutlineThickness(1.5f);
        d.setOrigin({r, r});
        d.setPosition({dotX, cy});
        window.draw(d);
        dotX -= (r * 2.f + 8.f);
    }
}

void Renderer::draw(const GameState& state) {
    window.clear(sf::Color(8, 8, 8));
    drawBoard(state);

    drawPieces(state);
    drawHUD(state);

    window.display();
}

void Renderer::setSelectedCell(const HexCell& cell) {
    m_selectedCell = cell;
    m_selectionPulseStart = std::chrono::steady_clock::now();
}

void Renderer::clearSelectedCell() {
    m_selectedCell.reset();
}

void Renderer::setHighlights(const std::vector<HexCell>& cells) {
    m_highlights = cells;
}

void Renderer::clearHighlights() {
    m_highlights.clear();
}

void Renderer::highlight(const HexCell& c) {
    m_highlights.push_back(c);
}

void Renderer::setStatusMessage(const std::string& message) {
    m_statusMessage = message;
}
