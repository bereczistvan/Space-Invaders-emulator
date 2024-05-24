cmake_minimum_required(VERSION 3.27)

project(space-invaders-emulator VERSION 1.0.0)

add_library(
    cpu8080
    cpu8080.c
    cpu8080.h
)
add_library(
    machine
    machine.c
    machine.h
)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

add_executable(invaders main.c)

target_link_libraries(invaders PRIVATE cpu8080)

target_link_libraries(invaders PRIVATE machine)

target_link_libraries(invaders PRIVATE SDL2::SDL2main)

target_link_libraries(invaders PRIVATE SDL2::SDL2)