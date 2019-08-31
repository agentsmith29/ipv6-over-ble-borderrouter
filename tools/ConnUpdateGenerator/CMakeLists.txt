cmake_minimum_required(VERSION 2.7)
project(ConnUpdateGenerator)

set(CMAKE_CXX_STANDARD 14)

add_executable(ConnUpdateGenerator main.cpp)

target_link_libraries(ConnUpdateGenerator ncurses)
