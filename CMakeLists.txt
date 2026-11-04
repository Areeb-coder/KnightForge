cmake_minimum_required(VERSION 3.16)
project(MyChessEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(chess_engine
    src/main.cpp
    src/Chess.cpp
    src/Phase1/Piece.cpp
    src/Phase2/Move.cpp
    src/UCI.cpp
)
target_include_directories(chess_engine PRIVATE src)

add_executable(perft_test
    tests/perft_test.cpp
    src/Chess.cpp
    src/Phase1/Piece.cpp
    src/Phase2/Move.cpp
)
target_include_directories(perft_test PRIVATE src)
enable_testing()
add_test(NAME perft_test COMMAND perft_test)

add_executable(san_test
    tests/san_test.cpp
    src/Chess.cpp
    src/Phase1/Piece.cpp
    src/Phase2/Move.cpp
)
target_include_directories(san_test PRIVATE src)
add_test(NAME san_test COMMAND san_test)
