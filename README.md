# Chess 3 Players — Yalta

Jeu d'échecs à 3 joueurs sur plateau hexagonal, développé en C++17 avec SFML 3.

## Prérequis

- [CMake](https://cmake.org/) ≥ 3.16
- [MinGW-w64](https://www.mingw-w64.org/) (g++ avec support C++17)
- [SFML 3.0](https://www.sfml-dev.org/) installé dans `C:/SFML`

## Compilation

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

L'exécutable `Chess3Players.exe` est généré dans le dossier `build/`.

## Lancement

```bash
.\build\Chess3Players.exe
```

## Utilisation

1. Dans le menu, choisir **Humain** ou **IA** pour chaque joueur
2. Choisir la difficulté de l'IA : **Facile** (prof. 1), **Normal** (prof. 2), **Difficile** (prof. 3)
3. Cliquer **Demarrer**

### Pendant la partie

| Action | Effet |
|--------|-------|
| Clic sur une pièce | Sélectionne la pièce et affiche ses coups légaux |
| Clic sur une case verte | Joue le coup |
| **↩ Annuler** (bouton bas-gauche) | Annule le dernier coup (2 coups si IA active) |
| Touche **I** | Affiche/masque les identifiants des cases |

## Architecture

```
src/
├── main.cpp
├── Model/        — logique du jeu (Board, GameState, pièces)
├── View/         — rendu SFML (Renderer, MenuRenderer)
└── Controller/   — gestion des événements (GameController)
```

## Patterns de conception

Le projet applique plusieurs patterns de conception classiques :

- **MVC (Modele-Vue-Controleur)** : separation entre la logique du jeu (`Model`), le rendu SFML (`View`) et la gestion des evenements utilisateur (`Controller`)
- **Factory** (`PieceFactory`) : centralise la creation des pieces d'echecs (pion, cavalier, fou, tour, dame, roi) et l'initialisation du plateau pour chaque joueur, evitant la duplication de logique de construction
- **Observer** (`IObserver`) : permet a la vue de se mettre a jour automatiquement lorsque l'etat du jeu change (`onStateChanged`), sans coupler directement le modele au rendu

Diagrammes disponibles dans le dossier `Diagramme/` : diagramme de classes, diagramme de cas d'utilisation, diagramme etats-transitions, et illustrations des patterns Factory et Observer.

## Fonction annuler (undo)

Chaque coup joue est enregistre dans une structure `Move` conservant l'etat necessaire pour le defaire (piece capturee, roque, prise en passant, promotion, statut de partie precedent). Le bouton **Annuler** restaure le dernier coup a partir de cet historique, deux coups d'affilee lorsque l'IA est active (le coup de l'IA puis celui du joueur).

## Algorithme IA

Minimax **paranoid** avec parallélisation `std::async` au niveau racine.  
Chaque joueur maximise son score ; les adversaires minimisent le score du joueur IA.
