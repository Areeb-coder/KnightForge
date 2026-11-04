#!/usr/bin/env bash
set -e
g++ -std=c++17 src/main.cpp src/Chess.cpp src/Phase1/Piece.cpp src/Phase2/Move.cpp src/UCI.cpp -O2 -o chess_engine
./chess_engine
