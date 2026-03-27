cmake_minimum_required(VERSION 3.16)
project(Chess3Players)

set(CMAKE_CXX_STANDARD 17)

# Chemin vers SFML
set(SFML_DIR "C:/SFML/lib/cmake/SFML")

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

add_executable(Chess3Players src/main.cpp)

target_link_libraries(Chess3Players PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)