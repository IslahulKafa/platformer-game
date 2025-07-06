cmake_minimum_required(VERSION 3.31)
project(platformer_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS graphics window system REQUIRED)

add_executable(platformer_game main.cpp
        src/Platform.cpp
        src/Platform.h
        src/Collectible.cpp
        src/Collectible.cpp
        src/Collectible.cpp
        src/Collectible.h
        src/Particle.cpp
        src/Particle.h
        src/obstacles/fire_launcher.cpp
        src/obstacles/fire_launcher.h
        src/Player.cpp
        src/Player.h)

target_link_libraries(platformer_game sfml-graphics sfml-window sfml-system)