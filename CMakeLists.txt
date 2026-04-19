cmake_minimum_required(VERSION 3.16)
project(Chess3Players)

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "C:/SFML/lib/cmake/SFML")
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

add_executable(Chess3Players
    src/main.cpp

    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp

    src/View/MenuRenderer.cpp
    src/View/Renderer.cpp

    src/Controller/GameController.cpp
)

target_link_libraries(Chess3Players PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(RendererPickTests
    src/renderer_pick_test.cpp

    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp
    src/View/Renderer.cpp
)

target_link_libraries(RendererPickTests PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(GameLogicManualTests
    src/game_logic_manual_test.cpp

    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp
)

target_link_libraries(GameLogicManualTests PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(MenuAIManualTests
    src/menu_ai_manual_test.cpp

    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp
)

target_link_libraries(MenuAIManualTests PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(ControllerManualTests
    src/controller_manual_test.cpp

    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp

    src/View/MenuRenderer.cpp
    src/View/Renderer.cpp

    src/Controller/GameController.cpp
)

target_link_libraries(ControllerManualTests PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(BoardMatrixDebug
    src/board_matrix_debug_main.cpp

    src/Model/boardaffichage.cpp
    src/Model/HexCell.cpp
    src/Model/Piece.cpp
    src/Model/Pawn.cpp
    src/Model/Knight.cpp
    src/Model/Bishop.cpp
    src/Model/Rook.cpp
    src/Model/Queen.cpp
    src/Model/King.cpp
    src/Model/Board.cpp
    src/Model/GameState.cpp
    src/Model/PieceFactory.cpp
)

target_link_libraries(BoardMatrixDebug PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)