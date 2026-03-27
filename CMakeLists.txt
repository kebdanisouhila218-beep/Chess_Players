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

    src/View/Renderer.cpp

    src/Controller/GameController.cpp
)

target_link_libraries(Chess3Players PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)