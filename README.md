# MyChessEngine — Phase 3

Phase 3 adds the complete chess-rules layer on top of your Phase 1 and Phase 2 board/move work.

## What Phase 3 contains

- Check detection
- Legal move filtering (moves that expose your own king are rejected)
- Checkmate
- Stalemate
- Castling on both sides
- En passant
- Pawn promotion to queen/rook/bishop/knight
- FEN load/save
- 50-move counter tracking
- Threefold repetition tracking
- Insufficient-material detection
- Make/undo move support
- Terminal two-player chess program
- Rule regression tests

## What Phase 3 does NOT contain

There is deliberately no computer opponent yet.
That belongs to Phase 4, where we add evaluation + minimax/alpha-beta search.

## Build — Windows / MSYS2

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Run:

```bash
./build/chess_phase3.exe
```

## Build — Linux / macOS

```bash
cmake -S . -B build
cmake --build build
./build/chess_phase3
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Terminal commands

```text
e2e4
moves
fen
setfen <FEN>
undo
new
help
quit
```

Promotion example:

```text
e7e8q
e7e8r
e7e8b
e7e8n
```

## Important milestone

When the Phase 3 tests pass, your program has a rules engine that can distinguish pseudo-legal moves from legal moves and correctly handle the major special chess rules.
