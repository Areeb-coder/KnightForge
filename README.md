# MyChessEngine — Pre-ML Final Build

This version intentionally stops **before machine learning / neural-network training**. It contains the complete terminal chess engine foundation and a classical search engine.

## What is included

### Phases 1–3: Chess foundation
- 8×8 board and pieces
- coordinate moves (`e2e4`)
- legal move generation
- check/checkmate/stalemate
- castling
- en passant
- promotion (`e7e8q`)
- FEN load/save
- make/undo
- 50-move and threefold-repetition tracking
- insufficient-material draw detection

### Phase 4: Classical search
- material + positional evaluation
- negamax/minimax-style search
- alpha-beta pruning
- move ordering
- quiescence search
- iterative deepening
- time control

### Phase 5: Strength improvements
- piece-square tables
- pawn-structure evaluation
- passed-pawn bonuses
- bishop-pair bonus
- rook file bonuses
- king-safety heuristics
- killer-move heuristic
- history heuristic
- transposition table

### Phase 6: Engine interface
- UCI mode (`--uci`)
- `uci`, `isready`, `position`, `go`, `ucinewgame`, `quit`
- can be connected to a UCI-compatible chess GUI later, without changing the engine core

### Phase 7: Diagnostics and testing
- Perft tests
- root `divide` command
- evaluation display
- search benchmark

## Build

### Windows (MSYS2/UCRT64)

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/chess_engine.exe
```

Or use the included `build_windows.bat` from a suitable MinGW environment.

### Linux/macOS

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/chess_engine
```

## Terminal commands

```text
e2e4                  make a move
moves                 list legal moves
fen                   print current FEN
setfen <FEN>          load a position
undo                  undo the last move
new                   new starting position
depth N               set search depth
time N                set thinking time in milliseconds
perft N               run perft
divide N               show per-root perft counts
eval                  show static evaluation
bench                 benchmark the current search settings
uci                   enter UCI mode
help                  show commands
quit                  exit
```

## UCI mode

Start directly with:

```bash
./build/chess_engine --uci
```

Or type `uci` inside the terminal program.

Example:

```text
uci
isready
position startpos moves e2e4 e7e5 g1f3
go depth 5
```

The engine replies with a `bestmove`.

## Verification

The included Perft test checks the standard starting-position counts:

- depth 1 = 20
- depth 2 = 400
- depth 3 = 8,902
- depth 4 = 197,281

## Deliberately not included yet

This build does **not** include:

- neural networks
- PyTorch
- reinforcement learning
- self-play training
- AlphaZero-style learning
- opening books
- endgame tablebases
- web frontend

Those are the next, separate stage after this pre-ML engine foundation.
