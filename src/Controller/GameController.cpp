#include "GameController.hpp"
#include "../Model/Pawn.hpp"

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
}

GameController::GameController()
    // 850 de hauteur = 800 pour le plateau + 50 pour le HUD joueur courant
    : window(sf::VideoMode({800, 850}), "Chess 3 Players")
    , renderer(window)
{
    state.addObserver(&renderer);
    renderer.setCurrentState(&state);

    factory.initBoard(state.getBoard(), Player::PLAYER1);
    factory.initBoard(state.getBoard(), Player::PLAYER2);
    factory.initBoard(state.getBoard(), Player::PLAYER3);

    renderer.draw(state);
}

std::string GameController::pieceLabel(const Piece* piece, const HexCell& cell) const {
    if (!piece) {
        return "Case vide";
    }
    return pieceTypeText(piece->getType()) + " (" + std::to_string(cell.q) + "," + std::to_string(cell.r) + ")";
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

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::View view(sf::FloatRect({0.f, 0.f}, {
                static_cast<float>(resized->size.x),
                static_cast<float>(resized->size.y)
            }));
            window.setView(view);
            renderer.draw(state);
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left)
                handleClick(click->position.x, click->position.y);
        }
    }
}

void GameController::handleClick(int x, int y) {
    // Ignore les clics dans la zone HUD (50px en bas)
    sf::Vector2u winSize = window.getSize();
    if (y >= static_cast<int>(winSize.y) - 50)
        return;

    std::optional<HexCell> picked = renderer.pickCell(state.getBoard(), {(float)x, (float)y});
    if (!picked.has_value()) {
        renderer.setStatusMessage("Aucune case selectionnee");
        if (selected != nullptr) {
            delete selected;
            selected = nullptr;

            validMoves.clear();
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
            renderer.clearHighlights();
            renderer.draw(state);
        }
        return;
    }

    if (selected == nullptr) {
        Piece* p = state.getBoard().getPiece(clicked);

        if (p && p->getOwner() == state.getCurrentPlayer()) {
            selected = new HexCell(clicked);

            if (p->getType() == PieceType::PAWN) {
                const Pawn* pawn = dynamic_cast<const Pawn*>(p);
                if (pawn)
                    validMoves = pawn->getMoves(state.getBoard(), state.getLastMove());
                else
                    validMoves = p->getMoves(state.getBoard());
            } else {
                validMoves = p->getMoves(state.getBoard());
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
            renderer.clearHighlights();
            renderer.draw(state);
        }
    } else {
        Piece* clickedPiece = state.getBoard().getPiece(clicked);

        if (clickedPiece && clickedPiece->getOwner() == state.getCurrentPlayer()) {
            *selected = clicked;
            if (clickedPiece->getType() == PieceType::PAWN) {
                const Pawn* pawn = dynamic_cast<const Pawn*>(clickedPiece);
                if (pawn)
                    validMoves = pawn->getMoves(state.getBoard(), state.getLastMove());
                else
                    validMoves = clickedPiece->getMoves(state.getBoard());
            } else {
                validMoves = clickedPiece->getMoves(state.getBoard());
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
            if (target == nullptr || target->getOwner() != state.getCurrentPlayer()) {
                renderer.setStatusMessage("Deplacement vers (" + std::to_string(clicked.q) + "," + std::to_string(clicked.r) + ")");
                renderer.clearHighlights();
                Move move{*selected, clicked, state.getCurrentPlayer()};
                state.applyMove(move);
            }
        } else {
            renderer.setStatusMessage("Destination non valide");
        }

        delete selected;
        selected = nullptr;
        validMoves.clear();
        renderer.clearHighlights();
        renderer.draw(state);
    }
}