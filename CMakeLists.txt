cmake_minimum_required(VERSION 3.29)
project(AutomatFinit)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES main.cpp Automaton.cpp)

add_executable(AutomatFinit main.cpp
        Automaton.cpp
        Automaton.h
        DFA.h
        DFA.cpp
        input.txt)
