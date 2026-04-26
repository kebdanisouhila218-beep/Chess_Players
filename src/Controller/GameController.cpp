#include "GameController.hpp"
#include "../Model/Pawn.hpp"
#include <iostream>

namespace {
    std::string pieceTypeText(PieceType type) {
        switch (type) {
            case PieceType::PAWN: return "Pion";
            case PieceType::KNIGHT: return "Cavalier";
            case PieceType::BISHOP: return "Fou";
            case PieceType::ROOK: return "Tour";
            case PieceType::QUEEN: return "Dame";
            case PieceType::KING: return "Roi";
            default: return "Piece";
        }
    }

    std::string playerName(Player player) {
        switch (player) {
            case Player::PLAYER1: return "Joueur 1 - Blancs";
            case Player::PLAYER2: return "Joueur 2 - Bleus";
            case Player::PLAYER3: return "Joueur 3 - Rouges";
            default: return "Aucun";
        }
    }

    std::string winnerText(Player player) {
        if (player == Player::NONE) return "Aucun gagnant";
        return playerName(player);
    }

    int playerIndex(Player player) {
        switch (player) {
            case Player::PLAYER1: return 0;
            case Player::PLAYER2: return 1;
            case Player::PLAYER3: return 2;
            default: return -1;
        }
    }

    std::string playerTag(Player player) {
        switch (player) {
            case Player::PLAYER1: return "J1-Blancs";
            case Player::PLAYER2: return "J2-Bleus";
            case Player::PLAYER3: return "J3-Rouges";
            default: return "??";
        }
    }

    std::string cellStr(const Board& b, const HexCell& c) {
        return "#" + std::to_string(b.getId(c)) + "(" + std::to_string(c.q) + "," + std::to_string(c.r) + ")";
    }
}

GameController::GameController(bool windowVisible)
    // 850 de hauteur = 800 pour le plateau + 50 pour le HUD joueur courant
    : window(sf::VideoMode({800, 850}), "Chess 3 Players")
    , menuRenderer(window)
    , renderer(window)
{
    window.setVisible(windowVisible);
    state.addObserver(&renderer);
    renderer.setCurrentState(&state);

    menuRenderer.draw(isAI, aiDepth);
}

void GameController::setAIConfig(const std::array<bool, 3>& config) {
    isAI = config;
    if (!gameStarted && window.isOpen()) {
        menuRenderer.draw(isAI, aiDepth);
    }
}

void GameController::setAIDifficulty(int depth) {
    aiDepth = depth;
    if (!gameStarted && window.isOpen()) {
        menuRenderer.draw(isAI, aiDepth);
    }
}

const std::array<bool, 3>& GameController::getAIConfig() const {
    return isAI;
}

void GameController::startGameForTests() {
    startGame();
}

bool GameController::stepAIMoveForTests(bool ignoreDelay) {
    return tryAIMove(ignoreDelay);
}

bool GameController::hasGameStarted() const {
    return gameStarted;
}

GameState& GameController::getState() {
    return state;
}

const GameState& GameController::getState() const {
    return state;
}

std::string GameController::pieceLabel(const Piece* piece, const HexCell& cell) const {
    if (!piece) {
        return "Case vide";
    }
    return pieceTypeText(piece->getType()) + " (" + std::to_string(cell.q) + "," + std::to_string(cell.r) + ")";
}

void GameController::run() {
    showMenu();
    if (!window.isOpen() || !gameStarted) {
        return;
    }

    while (window.isOpen()) {
        if (gameStarted) {
            tryAIMove();
        }
        handleEvents();
    }
}

void GameController::startGame() {
    gameStarted = true;
    lastAIMoveTime = std::chrono::steady_clock::now();
    renderer.setStatusMessage("Partie demarree");
    renderer.draw(state);
    tryAIMove();
}

void GameController::showMenu() {
    menuRenderer.draw(isAI, aiDepth);

    while (window.isOpen() && !gameStarted) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::View view(sf::FloatRect({0.f, 0.f}, {
                    static_cast<float>(resized->size.x),
                    static_cast<float>(resized->size.y)
                }));
                window.setView(view);
                menuRenderer.draw(isAI, aiDepth);
            }

            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left) {
                    handleMenuClick(click->position.x, click->position.y);
                }
            }
        }
    }
}

void GameController::handleMenuClick(int x, int y) {
    const sf::Vector2f mouse{static_cast<float>(x), static_cast<float>(y)};

    const auto toggleBounds = menuRenderer.getToggleBounds();
    for (std::size_t i = 0; i < toggleBounds.size(); ++i) {
        if (toggleBounds[i].contains(mouse)) {
            isAI[i] = !isAI[i];
            menuRenderer.draw(isAI, aiDepth);
            return;
        }
    }

    const auto diffBounds = menuRenderer.getDifficultyBounds();
    for (std::size_t i = 0; i < diffBounds.size(); ++i) {
        if (diffBounds[i].contains(mouse)) {
            aiDepth = static_cast<int>(i) + 1;
            menuRenderer.draw(isAI, aiDepth);
            return;
        }
    }

    if (menuRenderer.getStartButtonBounds().contains(mouse)) {
        startGame();
    }
}

void GameController::updateStatusMessage(const std::string& yaltaMessage) {
    if (state.isGameOver()) {
        renderer.setStatusMessage("Partie terminee - Gagnant : " + winnerText(state.getWinner()));
        return;
    }
    if (!yaltaMessage.empty()) {
        renderer.setStatusMessage(yaltaMessage);
        return;
    }
    switch (state.getStatus()) {
        case GameStatus::CHECK:
            renderer.setStatusMessage("Echec au roi !");
            break;
        case GameStatus::CHECKMATE:
            renderer.setStatusMessage("Echec et mat !");
            break;
        case GameStatus::DRAW:
            if (state.getHalfmoveClock() >= 50)
                renderer.setStatusMessage("Match nul - Regle des 50 coups");
            else
                renderer.setStatusMessage("Pat - Match nul !");
            break;
        default:
            break;
    }
}

bool GameController::tryAIMove(bool ignoreDelay) {
    if (state.isGameOver()) {
        return false;
    }

    const int currentIndex = playerIndex(state.getCurrentPlayer());
    if (currentIndex < 0 || !isAI[static_cast<std::size_t>(currentIndex)]) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!ignoreDelay && now - lastAIMoveTime < aiMoveDelay) {
        return false;
    }

    if (selected != nullptr) {
        delete selected;
        selected = nullptr;
    }
    validMoves.clear();
    renderer.clearSelectedCell();
    renderer.clearHighlights();
    renderer.setStatusMessage("IA reflechit...");

    std::optional<Move> bestMove = state.findBestMove(aiDepth, state.getCurrentPlayer());
    if (!bestMove.has_value()) {
        return false;
    }

    {
        const Board& b = state.getBoard();
        Piece* mp = b.getPiece(bestMove->from);
        std::string ptype = mp ? pieceTypeText(mp->getType()) : "?";
        state.applyMove(*bestMove);
        lastAIMoveTime = now;
        std::cout << "\n[IA-" << playerTag(bestMove->player) << "] "
                  << "joue " << ptype << " "
                  << cellStr(b, bestMove->from) << " -> " << cellStr(b, bestMove->to) << "\n";
    }

    {
        const Move* lm = state.getLastMove();
        std::string yMsg;
        if (lm && lm->yaltaEliminatedPlayer != Player::NONE) {
            yMsg = playerName(lm->yaltaEliminatedPlayer) + " elimine ! Pieces transferees a "
                 + playerName(state.getLastAttacker());
        }
        updateStatusMessage(yMsg);
    }

    renderer.draw(state);
    return true;
}

void GameController::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::View view(sf::FloatRect({0.f, 0.f}, {
                static_cast<float>(resized->size.x),
                static_cast<float>(resized->size.y)
            }));
            window.setView(view);
            if (gameStarted) {
                renderer.draw(state);
            } else {
                menuRenderer.draw(isAI, aiDepth);
            }
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::I && gameStarted) {
                renderer.toggleShowIds();
                renderer.draw(state);
            }
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left) {
                if (gameStarted) {
                    handleClick(click->position.x, click->position.y);
                } else {
                    handleMenuClick(click->position.x, click->position.y);
                }
            }
        }
    }
}

void GameController::handleClick(int x, int y) {
    if (state.isGameOver()) {
        renderer.setStatusMessage("Partie terminee - Gagnant : " + winnerText(state.getWinner()));
        if (selected != nullptr) {
            delete selected;
            selected = nullptr;
        }
        validMoves.clear();
        renderer.clearSelectedCell();
        renderer.clearHighlights();
        renderer.draw(state);
        return;
    }

    // Ignore les clics dans la zone HUD (50px en bas)
    sf::Vector2u winSize = window.getSize();
    if (y >= static_cast<int>(winSize.y) - 50)
        return;

    // Bouton Annuler
    const sf::Vector2f mousePos{static_cast<float>(x), static_cast<float>(y)};
    if (renderer.getUndoButtonBounds().contains(mousePos)) {
        if (!state.getMoveHistory().empty()) {
            const bool anyAI = isAI[0] || isAI[1] || isAI[2];
            const int movesToUndo = anyAI ? 2 : 1;
            const int available   = static_cast<int>(state.getMoveHistory().size());
            const int toUndo      = std::min(movesToUndo, available);
            if (selected != nullptr) {
                delete selected;
                selected = nullptr;
            }
            validMoves.clear();
            renderer.clearSelectedCell();
            renderer.clearHighlights();
            for (int i = 0; i < toUndo; ++i) {
                state.undoMove(i == toUndo - 1);
            }
            updateStatusMessage();
        }
        return;
    }

    std::optional<HexCell> picked = renderer.pickCell(state.getBoard(), {(float)x, (float)y});
    if (!picked.has_value()) {
        renderer.setStatusMessage("Aucune case selectionnee");
        if (selected != nullptr) {
            delete selected;
            selected = nullptr;

            validMoves.clear();
            renderer.clearSelectedCell();
            renderer.clearHighlights();
            renderer.draw(state);
        }
        return;
    }

    HexCell clicked = *picked;

    if (!state.getBoard().isValid(clicked)) {
        renderer.setStatusMessage("Case invalide");
        if (selected != nullptr) {
            delete selected;
            selected = nullptr;

            validMoves.clear();
            renderer.clearSelectedCell();
            renderer.clearHighlights();
            renderer.draw(state);
        }
        return;
    }

    if (selected == nullptr) {
        Piece* p = state.getBoard().getPiece(clicked);

        if (p && p->getOwner() == state.getCurrentPlayer()) {
            selected = new HexCell(clicked);
            renderer.setSelectedCell(clicked);
            validMoves = state.getLegalMoves(clicked);
            {
                const Board& b = state.getBoard();
                std::cout << "\n[" << playerTag(p->getOwner()) << "] "
                          << pieceTypeText(p->getType()) << " " << cellStr(b, clicked)
                          << " | " << validMoves.size() << " coup(s) possible(s) :";
                if (validMoves.empty()) {
                    std::cout << " aucun";
                } else {
                    for (const HexCell& m : validMoves)
                        std::cout << "  " << cellStr(b, m);
                }
                std::cout << "\n";
            }
            if (validMoves.empty()) {
                renderer.setStatusMessage(pieceLabel(p, clicked) + " : aucun coup disponible");
            } else {
                renderer.setStatusMessage(pieceLabel(p, clicked) + " : " + std::to_string(validMoves.size()) + " coups");
            }

            renderer.setHighlights(validMoves);
            renderer.draw(state);
        } else {
            if (p) {
                renderer.setStatusMessage(pieceLabel(p, clicked) + " : pas votre tour");
            } else {
                renderer.setStatusMessage("Case vide");
            }
            renderer.clearSelectedCell();
            renderer.clearHighlights();
            renderer.draw(state);
        }
    } else {
        Piece* clickedPiece = state.getBoard().getPiece(clicked);

        if (clickedPiece && clickedPiece->getOwner() == state.getCurrentPlayer()) {
            *selected = clicked;
            renderer.setSelectedCell(clicked);
            validMoves = state.getLegalMoves(clicked);
            {
                const Board& b = state.getBoard();
                std::cout << "\n[" << playerTag(clickedPiece->getOwner()) << "] "
                          << pieceTypeText(clickedPiece->getType()) << " " << cellStr(b, clicked)
                          << " | " << validMoves.size() << " coup(s) possible(s) :";
                if (validMoves.empty()) {
                    std::cout << " aucun";
                } else {
                    for (const HexCell& m : validMoves)
                        std::cout << "  " << cellStr(b, m);
                }
                std::cout << "\n";
            }
            if (validMoves.empty()) {
                renderer.setStatusMessage(pieceLabel(clickedPiece, clicked) + " : aucun coup disponible");
            } else {
                renderer.setStatusMessage(pieceLabel(clickedPiece, clicked) + " : " + std::to_string(validMoves.size()) + " coups");
            }

            renderer.setHighlights(validMoves);
            renderer.draw(state);
            return;
        }

        bool isValid = false;
        for (const HexCell& m : validMoves) {
            if (m == clicked) { isValid = true; break; }
        }

        if (isValid) {
            Piece* target = state.getBoard().getPiece(clicked);
            // Allow move to empty square OR capture of enemy piece
            if (target == nullptr || target->getOwner() != state.getCurrentPlayer()) {
                renderer.setStatusMessage("Deplacement vers (" + std::to_string(clicked.q) + "," + std::to_string(clicked.r) + ")");
                renderer.clearHighlights();
                Move move{*selected, clicked, state.getCurrentPlayer()};
                {
                    const Board& b = state.getBoard();
                    Piece* mp = b.getPiece(move.from);
                    std::string ptype = mp ? pieceTypeText(mp->getType()) : "?";
                    std::cout << "[" << playerTag(move.player) << "] "
                              << "joue " << ptype << " "
                              << cellStr(b, move.from) << " -> " << cellStr(b, move.to) << "\n";
                }
                state.applyMove(move);
                {
                    const Move* lm = state.getLastMove();
                    std::string yMsg;
                    if (lm && lm->yaltaEliminatedPlayer != Player::NONE) {
                        yMsg = playerName(lm->yaltaEliminatedPlayer) + " elimine ! Pieces transferees a "
                             + playerName(state.getLastAttacker());
                    }
                    updateStatusMessage(yMsg);
                }
            }
        } else {
            renderer.setStatusMessage("Destination non valide");
        }

        delete selected;
        selected = nullptr;
        validMoves.clear();
        renderer.clearSelectedCell();
        renderer.clearHighlights();
        renderer.draw(state);
        tryAIMove();
    }
}
