# MyChessEngine — Final Terminal Version

This version starts from the user's Phase 1 + Phase 2 project and completes the core terminal chess engine milestones.

## Included

### Phase 1–2
- Piece and color representation
- 8×8 board
- Coordinate moves such as `e2e4`

### Phase 3 — Complete chess rules
- Legal move generation
- Check detection
- Checkmate and stalemate
- Castling
- En passant
- Promotion
- FEN import/export
- 50-move counter
- Threefold repetition tracking

### Phase 4+ — Basic chess engine
- Material + simple positional evaluation
- Minimax/negamax alpha-beta search
- Move ordering
- Quiescence search
- Iterative deepening
- Time limit
- Transposition table
- Terminal engine opponent
- Perft testing

## Build with g++

From this project folder:

```bash
g++ -std=c++17 src/main.cpp src/Chess.cpp src/Phase1/Piece.cpp src/Phase2/Move.cpp -O2 -o chess_engine.exe
```

Run:

```bash
./chess_engine.exe
```

On Windows CMD/PowerShell:

```text
chess_engine.exe
```

## Build with CMake

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Commands

```text
e2e4       Make a move
moves      List all legal moves
fen        Show current FEN
setfen ... Load a FEN position
undo       Undo the last half-move
depth 4    Search depth cap
time 1000  Search time in milliseconds
perft 4    Count legal move tree nodes
new        New game
help       Show commands
quit       Exit
```

## Important note

This is intentionally a small educational engine, not a Stockfish-strength engine. The next major upgrade should be stronger evaluation and faster board representation (bitboards), followed later by ML.
