cmake_minimum_required(VERSION 3.28)
project(snake)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL3 REQUIRED CONFIG)

add_executable(snake main.cpp)

target_link_libraries(snake
    PRIVATE
    SDL3::SDL3
)


