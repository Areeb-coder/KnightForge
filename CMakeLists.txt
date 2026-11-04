cmake_minimum_required(VERSION 3.16)
project(MyChessEngine_Phase3 VERSION 0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

add_executable(chess_phase3
    src/main.cpp
    src/Board.cpp
    src/MoveGenerator.cpp
)

target_include_directories(chess_phase3 PRIVATE src)

include(CTest)
if(BUILD_TESTING)
    add_executable(phase3_tests
        tests/phase3_tests.cpp
        src/Board.cpp
        src/MoveGenerator.cpp
    )
    target_include_directories(phase3_tests PRIVATE src)
    add_test(NAME phase3_tests COMMAND phase3_tests)
endif()
