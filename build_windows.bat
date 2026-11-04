@echo off
g++ -std=c++17 src/main.cpp src/Chess.cpp src/Phase1/Piece.cpp src/Phase2/Move.cpp -O2 -o chess_engine.exe
if errorlevel 1 exit /b 1
chess_engine.exe
