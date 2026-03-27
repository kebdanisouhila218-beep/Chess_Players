#include "GameController.hpp"

GameController::GameController()
    : window(sf::VideoMode({800, 800}), "Chess 3 Players")
    , renderer(window)
{
    // Observer : renderer se met a jour automatiquement
    state.addObserver(&renderer);
    renderer.setCurrentState(&state);

    // Factory : initialise les pieces des 3 joueurs
    factory.initBoard(state.getBoard(), Player::PLAYER1);
    factory.initBoard(state.getBoard(), Player::PLAYER2);
    factory.initBoard(state.getBoard(), Player::PLAYER3);

    // Premier affichage
    renderer.draw(state);
}

void GameController::run() {
    while (window.isOpen()) {
        handleEvents();
    }
}

void GameController::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left)
                handleClick(click->position.x, click->position.y);
        }
    }
}

void GameController::handleClick(int x, int y) {
    HexCell clicked = renderer.pixelToHex({(float)x, (float)y});

    if (!state.getBoard().isValid(clicked)) return;

    // Aucune piece selectionnee
    if (selected == nullptr) {
        Piece* p = state.getBoard().getPiece(clicked);
        if (p && p->getOwner() == state.getCurrentPlayer()) {
            selected = new HexCell(clicked);
            validMoves = p->getMoves(state.getBoard());
            // Affiche les cases valides
            renderer.draw(state);
            for (const HexCell& m : validMoves)
                renderer.highlight(m);
            window.display();
        }
    } else {
        // Verifie si le clic est un mouvement valide
        bool isValid = false;
        for (const HexCell& m : validMoves) {
            if (m == clicked) { isValid = true; break; }
        }

        if (isValid) {
            Move move{*selected, clicked, state.getCurrentPlayer()};
            state.applyMove(move); // notifie automatiquement le renderer
        }

        delete selected;
        selected = nullptr;
        validMoves.clear();
    }
}